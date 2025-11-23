#include <stdio.h>
#include <stdlib.h>

// 隣接リスト
typedef  struct Edge {
    int nextto;         // 隣接駅のID
    double length;      // 線の距離
    struct Edge* next;  // 同じ始点を持つ次の辺のポインタ
} Edge;

// グラフ構造体
typedef struct {
    Edge** adj;         // 隣接リスト
    int n;              // 駅の総数
} Graph;

// 入力を一時保存
typedef struct {
    int start, end;     // 始点と終点の駅ID
    double length;      // 距離
} InputEdge;

// Edgeノードを動的に確保
Edge* new_edge(int nextto, double length, Edge* next) {
    Edge* e = (Edge*)malloc(sizeof(Edge)); //新しいEdgeノード（駅の経路）のためにメモリを確保
    if (e == NULL) {
        fprintf(stderr, "Memory allocation failed in new_edge.¥n"); exit(1);    //メモリ確保失敗時
    }
    e->nextto = nextto;
    e->length = length;
    e->next = next;
    return e;
}

//無向グラフとしてEdgeを追加
void add_edge(Graph* g, int start, int end, double length) {
    g->adj[start] = new_edge(end, length, g->adj[start]);   //start->endのEdgeを追加。新しいEdgeをstartの隣接リストの先頭に挿入
    g->adj[end] = new_edge(start, length, g->adj[end]);     //無向グラフであるため反対も
}

//最長片道の記録用
int* longest_path;          //最長の片道の駅を記録する
int longest_count = 0;        //最長の片道の駅数
double longest_dist = -1;   //最長の片道の距離
int* visited;   //訪問の判定
int* path;      //現在の経路

/* ----------------------
最長片道を探索  begin:始点 now:現在の駅
---------------------- */
void find_path(Graph* g, int begin, int now, int count, double dist) {
    visited[now] = 1;
    path[count] = now;

    //始点と終点が違う場合
    if (dist > longest_dist) {
        // printf("  -> 直線更新: dist %.2f > longest_dist %.2f\n", dist, longest_dist);
        longest_dist = dist;
        longest_count = count + 1;
        for (int i = 0; i < longest_count; i++) {
            longest_path[i] = path[i];
        }
    }

    //始点と終点が同じ場合
    for (Edge* e = g->adj[now]; e; e = e->next) {
        // 1→2→1のようなサイクル（往復を許す場合）
        // 往復がダメな場合if (e->nextto == begin && count >= 2)
        if (e->nextto == begin && count >= 1) {
            double cycle_dist = dist + e->length;
            if (cycle_dist > longest_dist) {
                // printf("    -> サイクル型更新: cycle_dist %.2f > longest_dist %.2f\n", cycle_dist, longest_dist);
                longest_dist = cycle_dist;
                longest_count = count + 2;
                for (int i = 0; i <= count; i++) {
                    longest_path[i] = path[i];
                }
                longest_path[count + 1] = begin;
            }
        }
    }

    //全ての駅を探索
    for (Edge* e = g->adj[now]; e; e = e->next) {
        if (!visited[e->nextto]) {
            find_path(g, begin, e->nextto, count + 1, dist + e->length);
        }
    }

    visited[now] = 0;   //一つ前の駅に戻る
}

int main() {
    InputEdge* edges = NULL;        //入力の配列
    int size = 0, capacity = 0;     //読み込んだ線の数、配列の容量
    int max_id = -1;                     //最大の駅のID

    //入力処理
    while (1) {
        int start, end;
        double length;
        int read = scanf(" %d , %d , %lf", &start, &end, &length);
        if (read == EOF || read < 3) break;

        //配列の容量を動的に確保
        if (size == capacity) {
            capacity = (capacity == 0 ? 16 : capacity * 2);
            InputEdge* tmp = (InputEdge *)realloc(edges, capacity * sizeof(InputEdge));
            if (tmp == NULL) {
                fprintf(stderr, "Memory allocation failed during realloc.¥n");
                free(edges);
                exit(1);    //メモリ確保失敗時
            }
            edges = tmp;
        }

        edges[size].start = start;
        edges[size].end = end;
        edges[size].length = length;
        size++;

        if (start > max_id) max_id = start;
        if (end > max_id) max_id = end;
    }

    //グラフ構築
    Graph g;
    g.n = max_id + 1;   //連番対応、非連番の場合メモリ大きくなる可能性あり
    g.adj = (Edge **)calloc(g.n, sizeof(Edge*));

    for (int i = 0; i < size; i++) {
        add_edge(&g, edges[i].start, edges[i].end, edges[i].length);
    }
    free(edges);

    //配列確保
    visited = (int *)calloc(g.n, sizeof(int));
    path = (int *)malloc(g.n * sizeof(int));
    longest_path = (int *)malloc((g.n + 1) * sizeof(int));  //cycle用+1

    //探索
    for (int i = 0; i < g.n; i++) {
        find_path(&g, i, i, 0, 0);
    }
    //出力
    for (int i = 0; i < longest_count; i++) {
        printf("%d\r\n", longest_path[i]);
    }

    //メモリ解法
    for (int i = 0; i < g.n; i++) {
        Edge* e = g.adj[i];
        while (e) {
            Edge* next = e->next;
            free(e);
            e = next;
        }
    }
    free(g.adj);
    free(visited);
    free(path);
    free(longest_path);

    return 0;
}