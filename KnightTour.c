#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// can be made local if required
int N;
typedef struct
{
    int x, y;
} pair;

//8 threads, and 100 x 100 vis array for each
int vis[8][100][100];

//to calculate the jumps basically, used to check if we have covered all squares
int steps[100];
struct args
{
    int a, b, idx;
};

//path found
int found = -1;

//for tie-breaking
struct dist_from_center
{
    int dist;
    pair p;
};

int isValid(int x, int y, int i)
{
    return x >= 0 && x < N && y >= 0 && y < N && !vis[i][x][y];
}

int board[101][101];

void print_path(pair path[], int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("%d,%d|", path[i].x, path[i].y);
    }
}

//possible directions of travel of the knight
int dir_x[8] = {1, 1, 2, 2, -1, -1, -2, -2};
int dir_y[8] = {2, -2, 1, -1, 2, -2, 1, -1};

//an array storing the coordinates of the next 8 possible squares of the knight, initialize with -1s
struct possibilities
{
    int square[8][2];
};

//there might be cases when not all next 8 possible squares are viable, hence we use get_len to find the number of VIABLE options
int get_len(struct possibilities p)
{
    int ans = 0;
    for (int i = 0; i < 8; i++)
    {
        if (p.square[i][0] == -1)
            break;
        ans++;
    }
    return ans;
}

struct possibilities get_possibilites(int x, int y, int idx)
{
    struct possibilities possible_moves;

    //intialisation
    for (int i = 0; i < 8; i++)
    {
        possible_moves.square[i][0] = -1;
        possible_moves.square[i][1] = -1;
    }

    int counter = 0; //total number of viable options
    for (int i = 0; i < 8; i++)
    {
        //not visited && in limits
        if (x + dir_x[i] >= 0 && x + dir_x[i] < N && y + dir_y[i] >= 0 && y + dir_y[i] < N && vis[idx][x + dir_x[i]][y + dir_y[i]] == 0)
        {
            possible_moves.square[counter][0] = x + dir_x[i];
            possible_moves.square[counter][1] = y + dir_y[i];
            counter++;
        }
    }
    return possible_moves;
}


//used to calculate the EUCLIDEAN distance used in tie breaking
int calculate_distance(int a, int b)
{
    return (N / 2 - a) * (N / 2 - a) + (N / 2 - b) * (N / 2 - b);
}

//used in sorting distances of squares with same degree in descending order --> choose the one farthest to the center first
int comparator(const void *s1, const void *s2)
{
    struct dist_from_center *e1 = (struct dist_from_center *)s1;
    struct dist_from_center *e2 = (struct dist_from_center *)s2;
    return e2->dist - e1->dist;
}

void solve(int a, int b, int idx)
{
    //0-indexed numbering of squares; idx is thread number
    if (found != -1)
        pthread_exit(NULL); // if found, exit
    steps[idx]++; //increase number of jumps by 1

    //the visited stores the numbering of square at (a,b) done by thread number idx
    vis[idx][a][b] = steps[idx] + 1;

    // to check if all squares have been visited.
    if (steps[idx] == N * N - 1)
    {
        //if a path is found, ie I have visited all the squares then found stores the value of the thread number which found the path
        found = idx;
        pthread_exit(NULL);
        return;
    }
    struct possibilities possible_moves = get_possibilites(a, b, idx);
    int len = get_len(possible_moves);

    //we use min_deg in deciding the minimum degree square to jump to, in accordance to Warnsdorff
    int min_deg = 9;
    //at max there can be all 8 next possible moves to have same minimum degree, dfc array stores their respective distances along with coordinates
    struct dist_from_center dfc[8];

    // number of elements in dfc array. (number of elements with same and minimum degree)
    int dfc_counter = 0;
    for (int i = 0; i < len; i++)
    {
        // to calculate degree of next square
        int curr_len = get_len(get_possibilites(possible_moves.square[i][0], possible_moves.square[i][1], idx));
        //I found a square with lesser degree, hence that is my best option to jump to
        if (curr_len < min_deg)
        {
            min_deg = curr_len;
            dfc_counter = 0;
            dfc[dfc_counter].dist = calculate_distance(possible_moves.square[i][0], possible_moves.square[i][1]);
            dfc[dfc_counter].p.x = possible_moves.square[i][0];
            dfc[dfc_counter].p.y = possible_moves.square[i][1];
            dfc_counter++;
        }
        //if I found a node with min deg itself, I will proceed to tie break by checking dfc
        else if (curr_len == min_deg)
        {
            dfc[dfc_counter].dist = calculate_distance(possible_moves.square[i][0], possible_moves.square[i][1]);
            dfc[dfc_counter].p.x = possible_moves.square[i][0];
            dfc[dfc_counter].p.y = possible_moves.square[i][1];
            dfc_counter++;
        }
        //if node greater than min deg, then just skip, not useful for us
    }

    //to sort in descending order of dfc of same degree nodes -- tie breaking
    qsort(dfc, dfc_counter, sizeof(struct dist_from_center), comparator);

    //But it is still possible that even after this tie break, I do not find a possible tour; but had I 
    //followed a different node in tie break, I could have found the answer. Hence we backtrack as well.
    for (int i = 0; i < dfc_counter; i++)
    {
        solve(dfc[i].p.x, dfc[i].p.y, idx);
    }
    vis[idx][a][b] = 0;
    steps[idx]--;
}

void *helper(void *a)
{
    struct args *b = (struct args *)(a);
    // printf("%d",b->idx);
    if (!isValid(b->a, b->b, b->idx))
        return NULL;
    solve(b->a, b->b, b->idx);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./Knight.out grid_size StartX StartY");
        exit(-1);
    }

    N = atoi(argv[1]);
    int StartX = atoi(argv[2]);
    int StartY = atoi(argv[3]);

    //creation of the initial 8 threads
    pthread_t tid[8];
    struct args a[8];

    //this is to measure the time, and use this information to smartly kill the thread if it exceeds a pre-determined value
    clock_t t, x;
    t = clock();
    for (int i = 0; i < 8; i++)
    {
        // vis- N*N matrix
        vis[i][StartX][StartY] = 1;
        a[i].a = StartX + dir_x[i];
        a[i].b = StartY + dir_y[i];
        a[i].idx = i;
        pthread_create(&tid[i], NULL, helper, (void *)&a[i]);
    }
    double time_taken;
    for (int i = 0; i < 8; i++)
    {
        while (1)
        {
            if (found != -1)
                break;
            x = clock() - t;
            time_taken = ((double)x) / CLOCKS_PER_SEC;
            if (time_taken > 1.0)
                break;
            // printf("%f", time_taken);
        }
        if (time_taken > 1.0)
        {
            pthread_cancel(tid[i]);
            continue;
        }
        pthread_join(tid[i], NULL);
    }

    if (found != -1)
    {
        //path is found -- store the path
        pair ans[N * N];
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                ans[vis[found][i][j] - 1].x = i;
                ans[vis[found][i][j] - 1].y = j;
            }
        }
        for (int i = 0; i < N * N; i++)
            printf("%d,%d|", ans[i].x, ans[i].y);
        printf("\n");
    }
    else
    {
        printf("No Possible Tour");
    }

    return 0;
}
