// Author：人工智慧導論 第一組
// Date：Dec.18,2025
// Purpose：
/*  - step = 一個時間單位
    - 地圖大小 = H × W 的網格
    - 每一隻貓都是獨立個體，有位置、顏色、飢餓、年齡與存活狀態
    - 食物是有週期的生成、隨時間腐敗、消失
    - 貓會移動、吃食物、繁殖、成長、因飢餓死亡
    - 繁殖與分化加入讓族群平衡，避免單一顏色的貓滅絕    
*/

#include <bits/stdc++.h>
#include <thread>
#include <chrono>
using namespace std;

// == 地圖 ==
const int H = 60;   // 地圖 Height
const int W = 250;  // 地圖 Width

const int STEPS = 20000;    // 第幾輪
const int SLEEP_MS = 60;

// == 食物 ==
const int FOOD_EVENTS = 14; // 每次生成的食物圈數
const int FOOD_PERIOD = 6;  // 每隔多久生成一次食物
const int FOOD_STALE_TIME = 40; // 食物過多久 -> 腐敗
const int FOOD_DECAY_TIME = 70; // 食物過多久 -> 消失

// == 移動 ==
const int SENSE_R = 26; // 搜尋食物的區域半徑
const int HERD_R  = 4;  // 群聚時的半徑
const double P_FOLLOW = 0.75;   // 跟隨群體移動的機率

const int SPD_BLUE = 1; // 藍貓每 step 會移動幾次
const int SPD_RED  = 2; // 紅貓每 step 會移動幾次

// == 飢餓死亡門檻 ==
// 當 hunger > 門檻，貓會死亡
const int H_BLUE   = 65;
const int H_RED    = 50;
const int H_PURPLE = 35;

// == 長大 ==
const int GROW_AGE = 40;    // 紫貓長大所需年齡
const double P_GROW_BLUE_BASE  = 0.6;   // 紫貓長大後變藍貓的機率

// == 生態系繁殖參數 ==

// 繁殖後的冷卻時間，避免連續生產
const int MATE_COOLDOWN = 45;

// 繁殖的最大飢餓值
const int MATE_HUNGER_LIMIT_BLUE = 20;
const int MATE_HUNGER_LIMIT_RED  = 18;

// 判定「附近有食物」的半徑
const int MATE_FOOD_R = 3;

// 基礎繁殖成功率
const double P_MATE_BASE = 0.18;

// 若附近有食物，額外加成的成功率
const double P_MATE_FOOD_BONUS = 0.22;

// 九宮格內最大允許活體數，太擁擠就不生
const int LOCAL_CROWD_LIMIT = 6;

// 每一個 step 全域最多允許出生幾隻
const int MAX_BIRTHS_PER_STEP = 20;

// == 族群平衡參數 ==
// 補強強度，越大代表補償效果越明顯
const double BALANCE_STRENGTH = 0.9;

// 當藍或紅比例低於這個值時，啟動強力補強
const double MIN_RATIO = 0.15;

// == 隨機工具 ==

// 隨機數引擎
mt19937 rng(random_device{}());

// 產生整數亂數 [a, b]
int irand(int a,int b){
    return uniform_int_distribution<int>(a,b)(rng);
}

// 產生 [0,1) 浮點亂數
double drand(){
    return uniform_real_distribution<double>(0,1)(rng);
}

// 暫停
void sleep_ms(int ms){
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// ANSI 控制游標位置（不清畫面）
void gotoxy(int x,int y){
    cout << "\x1b[" << y << ";" << x << "H";
}

// == 顯示顏色工具 ==
// 使用 ANSI escape code 讓不同角色顯示不同顏色

string blue(const string&s){ return "\x1b[34m"+s+"\x1b[0m"; }
string red(const string&s){ return "\x1b[31m"+s+"\x1b[0m"; }
string purple(const string&s){ return "\x1b[35m"+s+"\x1b[0m"; }
string yellow(const string&s){ return "\x1b[33m"+s+"\x1b[0m"; }
string brown(const string&s){ return "\x1b[38;5;94m"+s+"\x1b[0m"; }
string gray(const string&s){ return "\x1b[90m"+s+"\x1b[0m"; }

// == 資料 ==
enum Color { BLUE, RED, PURPLE };   // 貓的顏色狀態
enum FoodState { FRESH, STALE };    // 食物狀態

struct Cat{
    int x,y;           // 所在位置
    Color c;           // 顏色（族群）
    int hunger=0;      // 飢餓值（每 step 增加）
    int age=0;         // 年齡
    bool alive=true;   // 是否存活
    int mate_cd=0;     // 繁殖冷卻計時
};

// 地圖每一格，只記錄食物索引
struct Cell{
    int food_id = -1;  // -1 表示沒有食物
};

struct FoodEvent{
    vector<pair<int,int>> cells; // 覆蓋的格子
    int born_step;               // 生成時間
    FoodState state = FRESH;     // 新鮮或腐敗
    bool alive = true;           // 是否仍存在
};

// == 全域狀態 ==

vector<Cat> cats;                              // 所有貓
vector<vector<Cell>> grid(H, vector<Cell>(W));  // 地圖
vector<FoodEvent> foods;                       // 所有食物

// 四方向移動向量
int dx4[4]={1,-1,0,0};
int dy4[4]={0,0,1,-1};

// 邊界
bool inb(int x,int y){
    return x>=0 && x<W && y>=0 && y<H;
}
// == 初始化 ==
void init(){
    // 清空所有貓與食物資料，重新開始模擬
    cats.clear();
    foods.clear();

    // 將整張地圖的 food_id 重設為 -1，表示一開始沒有任何食物
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
            grid[y][x].food_id = -1;

    // 貓的初始數量隨機( 380 ~ 480)
    for(int i=0;i<irand(380,480);i++)
        cats.push_back({irand(0,W-1), irand(0,H-1), BLUE});

    for(int i=0;i<irand(380,480);i++)
        cats.push_back({irand(0,W-1), irand(0,H-1), RED});
}

// == 生食物 ==
void spawn_food(int step){
    // 只有在 step 為 FOOD_PERIOD 的倍數時才生成食物
    if(step % FOOD_PERIOD) return;

    // 每次生成 FOOD_EVENTS 團食物
    for(int i=0;i<FOOD_EVENTS;i++){
        // 隨機選一個中心點（避免太靠近邊界）
        int cx = irand(2,W-3);
        int cy = irand(2,H-3);

        FoodEvent fe;
        fe.born_step = step; // 記錄生成時間，用於老化判斷

        // 食物的形狀是「菱形」
        // Manhattan distance <= 2 的所有格子都屬於這團食物
        for(int dy=-2; dy<=2; dy++){
            for(int dx=-2; dx<=2; dx++){
                if(abs(dx)+abs(dy)<=2){
                    int nx=cx+dx, ny=cy+dy;
                    if(inb(nx,ny)){
                        // grid 只記錄該格屬於哪一個 food event
                        grid[ny][nx].food_id = (int)foods.size();
                        fe.cells.push_back({nx,ny});
                    }
                }
            }
        }
        foods.push_back(fe);
    }
}

// == 食物老化 ==
void update_foods(int step){
    for(auto &f:foods){
        if(!f.alive) continue;

        // 計算這團食物已存在多久
        int age = step - f.born_step;

        // 存在時間超過 FOOD_DECAY_TIME -> 食物完全消失
        if(age >= FOOD_DECAY_TIME){
            f.alive = false;
            for(auto&p:f.cells)
                grid[p.second][p.first].food_id = -1;
        }
        // 存在時間超過 FOOD_STALE_TIME -> 食物變成腐敗狀態
        else if(age >= FOOD_STALE_TIME){
            f.state = STALE;
        }
    }
}

// == 群聚方向 ==
pair<int,int> herd_dir(const Cat& c){
    int vx=0, vy=0, cnt=0;

    // 掃描 HERD_R 範圍內的所有活貓
    for(auto &o:cats){
        if(!o.alive) continue;
        if(abs(o.x-c.x)<=HERD_R && abs(o.y-c.y)<=HERD_R){
            // 累積鄰居相對位置，形成群體趨勢方向
            vx += (o.x - c.x);
            vy += (o.y - c.y);
            cnt++;
        }
    }

    // 鄰居數太少時，不形成群聚行為
    if(cnt < 3) return {0,0};

    // 回傳主導方向（x 或 y）
    return (abs(vx)>abs(vy)) ?
        make_pair(vx>0?1:-1,0) :
        make_pair(0,vy>0?1:-1);
}

// == 找食物 ==
pair<int,int> food_dir(const Cat& c){
    int best=INT_MAX, tx=c.x, ty=c.y;

    // 在 SENSE_R 視野內搜尋最近的食物
    for(int dy=-SENSE_R;dy<=SENSE_R;dy++)
        for(int dx=-SENSE_R;dx<=SENSE_R;dx++){
            int nx=c.x+dx, ny=c.y+dy;
            if(!inb(nx,ny)) continue;

            int id = grid[ny][nx].food_id;
            if(id==-1 || id>= (int)foods.size() || !foods[id].alive) continue;

            int d = abs(dx)+abs(dy);
            if(d<best){
                best=d;
                tx=nx; ty=ny;
            }
        }

    // 若找到食物，回傳「往該食物前進一步」的方向
    if(best!=INT_MAX){
        if(abs(tx-c.x)>abs(ty-c.y))
            return {tx>c.x?1:-1,0};
        else
            return {0,ty>c.y?1:-1};
    }
    return {0,0};
}

// == 決定移動 ==
pair<int,int> move_dir(const Cat& c){
    // 若視野內有食物，優先朝食物移動
    auto fd = food_dir(c);
    if(fd.first||fd.second) return fd;

    // 否則以 P_FOLLOW 機率跟隨群體方向
    if(drand()<P_FOLLOW){
        auto hd = herd_dir(c);
        if(hd.first||hd.second) return hd;
    }

    // 沒有時隨機移動
    int k=irand(0,3);
    return {dx4[k],dy4[k]};
}

// == 判斷附近是否有食物 ==
bool has_food_near(int x,int y,int R){
    for(int dy=-R; dy<=R; dy++){
        for(int dx=-R; dx<=R; dx++){
            int nx=x+dx, ny=y+dy;
            if(!inb(nx,ny)) continue;

            // 若有食物，回傳 true
            int id = grid[ny][nx].food_id;
            if(id!=-1 && id<(int)foods.size() && foods[id].alive)
                return true;
        }
    }
    // 沒食物
    return false;
}

// == 位置索引 ==
struct LocalInfo{
    int alive_cnt=0;          // 該格活貓數量
    bool has_red=false;       // 該格是否至少存在一隻紅貓
    int red_min_hunger=INT_MAX; // 該格中紅貓的最小 hunger
};

vector<vector<LocalInfo>> local_map;

// 掃描所有活貓，將其資訊累積到對應的格子
void build_local_map(){
    local_map.assign(H, vector<LocalInfo>(W));

    for(auto &c:cats){
        if(!c.alive) continue;

        auto &L = local_map[c.y][c.x];
        L.alive_cnt++;

        // 只特別記錄紅貓，因為紅貓是繁殖的必要條件之一
        if(c.c==RED){
            L.has_red=true;
            L.red_min_hunger = min(L.red_min_hunger, c.hunger);
        }
    }
}

// == 九宮格內資訊 ==
LocalInfo scan_3x3(int x,int y){
    LocalInfo out;
    out.red_min_hunger = INT_MAX;

    for(int dy=-1; dy<=1; dy++){
        for(int dx=-1; dx<=1; dx++){
            int nx=x+dx, ny=y+dy;
            if(!inb(nx,ny)) continue;

            auto &L = local_map[ny][nx];
            out.alive_cnt += L.alive_cnt;

            // 只要九宮格內任一格有紅貓，就算存在紅貓
            if(L.has_red){
                out.has_red = true;
                out.red_min_hunger = min(out.red_min_hunger, L.red_min_hunger);
            }
        }
    }
    return out;
}

// == 出生位置選擇 ==
pair<int,int> pick_birth_cell(int x,int y){
    vector<pair<int,int>> cand;
    cand.reserve(9);

    // 收集九宮格內所有合法位置
    for(int dy=-1; dy<=1; dy++){
        for(int dx=-1; dx<=1; dx++){
            int nx=x+dx, ny=y+dy;
            if(inb(nx,ny)) cand.push_back({nx,ny});
        }
    }

    // 找出目前最不擁擠的格子
    int best = INT_MAX;
    for(auto &p:cand){
        best = min(best, local_map[p.second][p.first].alive_cnt);
    }

    // 可能有多個同樣最少的格子，從中隨機挑一個
    vector<pair<int,int>> bestCand;
    for(auto &p:cand){
        if(local_map[p.second][p.first].alive_cnt == best)
            bestCand.push_back(p);
    }

    return bestCand[irand(0, (int)bestCand.size()-1)];
}

// == 演化 ==
void step_sim(int step){
    // 生成新食物
    spawn_food(step);

    // 更新食物狀態
    update_foods(step);

    build_local_map();

    // 4. 計算藍紅貓比例
    int cntB=0, cntR=0;
    for(auto &c:cats){
        if(!c.alive) continue;
        if(c.c==BLUE) cntB++;
        else if(c.c==RED) cntR++;
    }
    double totalBR = max(1, cntB + cntR);
    double blue_ratio = cntB / totalBR;
    double red_ratio  = cntR / totalBR;

    // 限制每 step 的出生數，避出生太多
    int births_this_step = 0;

    int original_n = (int)cats.size();

    for(int i=0;i<original_n;i++){
        auto &c = cats[i];
        if(!c.alive) continue;

        // 繁殖冷卻
        if(c.mate_cd > 0) c.mate_cd--;

        // 6. 根據顏色決定移動速度
        int spd = (c.c==RED?SPD_RED:SPD_BLUE);
        for(int s=0;s<spd;s++){
            auto[mx,my]=move_dir(c);
            int nx=c.x+mx, ny=c.y+my;
            if(inb(nx,ny)) c.x=nx, c.y=ny;
        }

        c.hunger++;
        c.age++;

        // 若吃到食物，重置 hunger
        int id = grid[c.y][c.x].food_id;
        if(id!=-1 && id<(int)foods.size() && foods[id].alive){
            grid[c.y][c.x].food_id = -1;
            c.hunger = 0;
        }
        // 繁殖
        if(c.c == BLUE && births_this_step < MAX_BIRTHS_PER_STEP){
            if(c.mate_cd == 0 && c.hunger <= MATE_HUNGER_LIMIT_BLUE){

                LocalInfo info = scan_3x3(c.x, c.y);

                if(info.has_red && info.alive_cnt <= LOCAL_CROWD_LIMIT){
                    if(info.red_min_hunger <= MATE_HUNGER_LIMIT_RED){

                        // 計算繁殖成功率
                        double p = P_MATE_BASE;

                        // 附近有食物會提高成功率
                        if(has_food_near(c.x, c.y, MATE_FOOD_R))
                            p += P_MATE_FOOD_BONUS;

                        // 族群平衡補強：若藍或紅過少，成功率上調
                        double balance_factor = 1.0;
                        if(blue_ratio < MIN_RATIO || red_ratio < MIN_RATIO){
                            double lack = max(0.0, MIN_RATIO - min(blue_ratio, red_ratio));
                            balance_factor += BALANCE_STRENGTH * (lack / MIN_RATIO);
                        }

                        p *= balance_factor;
                        p = min(p, 0.95); // 避免機率過高

                        // 擲骰決定是否成功繁殖
                        if(drand() < p){
                            auto [bx, by] = pick_birth_cell(c.x, c.y);
                            cats.push_back({bx, by, PURPLE});
                            births_this_step++;
                            c.mate_cd = MATE_COOLDOWN;
                        }
                    }
                }
            }
        }

        // 依族群比例決定變成藍或紅
        if(c.c==PURPLE && c.age>=GROW_AGE){
            double p_blue = P_GROW_BLUE_BASE;

            // 若藍較少，增加變藍機率
            double diff = (red_ratio - blue_ratio);
            p_blue += BALANCE_STRENGTH * diff;

            // 保護上下限
            p_blue = min(0.95, max(0.05, p_blue));

            c.c = (drand() < p_blue) ? BLUE : RED;
            c.age=0;

            // 分化後給一個隨機冷卻
            c.mate_cd = irand(0, MATE_COOLDOWN/2);
        }

        // 飢餓死亡
        int limit = (c.c==BLUE?H_BLUE:(c.c==RED?H_RED:H_PURPLE));
        if(c.hunger > limit) c.alive=false;
    }
}

// == 畫面 ==
void render(int step){
    // ===== 統計用變數 =====
   
    int b=0,r=0,p=0;     // b, r, p = 目前存活的藍 / 紅 / 紫貓數量
    int fresh=0,stale=0;    // fresh, stale = 新鮮 / 腐敗的食物數量

    // 計算活貓數量
    for(auto&c:cats){
        if(!c.alive) continue;

        if(c.c==BLUE)      b++;
        else if(c.c==RED)  r++;
        else               p++;
    }

    // 計算食物數量
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++){
            int id = grid[y][x].food_id;
            if(id!=-1 && id<(int)foods.size() && foods[id].alive){
                if(foods[id].state==FRESH) fresh++;
                else                       stale++;
            }
        }

    // == 第一列：狀態列 ==
    gotoxy(1,1);
    cout << "\x1b[2K";   // 清空該行，避免殘影
    cout << "Step " << step
         << " | Cats Total " << (b+r+p)
         << " B:" << b << " R:" << r << " P:" << p
         << " | Food Total " << (fresh+stale)
         << " Fresh:" << fresh << " Stale:" << stale;
    
    // == 第二列：清空行 ==
    gotoxy(1,2);
    cout << "\x1b[2K";

    // == 建立地圖顯示用暫存資料 ==
    vector<vector<int>> cnt(H,vector<int>(W,0));            // 一格有多少隻活貓(會重疊)
    vector<vector<Color>> top(H,vector<Color>(W,BLUE));     // 一格最上層要顯示的貓顏色

    // 將所有活著的貓投影到畫面格子上
    for(auto&c:cats){
        if(!c.alive) continue;
        cnt[c.y][c.x]++;
        top[c.y][c.x]=c.c;
    }

    // 第三列：地圖
    for(int y=0;y<H;y++){
        gotoxy(1,y+3);
        for(int x=0;x<W;x++){

            // 優先顯示貓
            if(cnt[y][x]){
                if(top[y][x]==BLUE)      cout<<blue("#");
                else if(top[y][x]==RED)  cout<<red("#");
                else                     cout<<purple("#");
            }
            // 如果沒貓，才顯示食物
            else if(grid[y][x].food_id!=-1){
                int id = grid[y][x].food_id;
                if(id>=0 && id<(int)foods.size() && foods[id].alive){
                   
                    if(foods[id].state==FRESH) cout<<yellow("@");    // 新鮮食物符號
                    else                       cout<<brown("%");     // 腐敗食物符號
                }else{
                    cout<<gray(".");    // 空地符號
                }
            }
            // 完全沒有東西的空地
            else cout<<gray(".");
        }
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "\x1b[2J";   // 清空終端機畫面
    init(); // 初始化環境

    for(int step=1;step<=STEPS;step++){
        step_sim(step);   // 生態演化
        render(step);     // 顯示當下生態
        sleep_ms(SLEEP_MS); // 播放速度
    }
    return 0;
}
