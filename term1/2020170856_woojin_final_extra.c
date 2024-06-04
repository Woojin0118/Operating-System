#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

// 프로세스 구조체
typedef struct _process {
    int pid;
    int cpu_burst;
    int io_burst;
    int arrive_time;
    int priority;
    int waiting_time;
    int turnaround_time;
    int remaining_time; // preemptive scheduling 때 필요
    int ready_completed;
    int io_remaining_time;
    int io_start_time;
} Process;

// Queue의 index에서 사용
typedef int Data;

// node 구조체
typedef struct _node {
    Data data;
    struct _node *next;
} Node;

// queue 구조체
typedef struct _queue {
    Node *front;
    Node *rear;
} Queue;

// Waiting Queue 구조체
typedef Queue WaitingQueue;

// Ready Queue 구조체
typedef Queue ReadyQueue;

// Gantt chart 구조체
typedef Queue GanttQueue;

void Create_Process(Process *p, int len);
void FCFS(Process *p, int len);
void SJF(Process *p, int len);
void Priority(Process *p, int len);
void Preem_SJF(Process *p, int len);
void Preem_Priority(Process *p, int len);
void RoundRobin(Process *p, int len, int quantum);
void RoundRobin_priority(Process *p, int len, int quantum);

void print_table(Process p[], int n);
void evaluate(Process p[], int n);

// 큐의 크기를 반환하는 함수
size_t getQueueSize(Queue* queue) {
    size_t size = 0;
    Node* current = queue->front;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

// queue 초기화 함수
void initQueue(Queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

int isEmpty(Queue *queue) {
    if (queue->front == NULL) return TRUE;
    else return FALSE;
}

// ready queue에 프로세스 삽입 함수
void enqueue(Queue *queue, Data data) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    
    if (isEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

Data dequeue(Queue *queue) {
    Data retdata;
    if (isEmpty(queue)) {
        exit(1);
    }
    Node *temp = queue->front;
    retdata = temp->data;
    queue->front = temp->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return retdata;
}

void Create_Process(Process *p, int len) {
    srand(time(NULL)); // 난수 생성을 매번 다른 seed값으로
    
    for (int i = 0; i < len; i++) {
        printf("Enter PID, Burst Time, Arrival Time, Priority (separated by spaces): ");
        scanf("%d %d %d %d", &p[i].pid, &p[i].cpu_burst, &p[i].arrive_time, &p[i].priority);
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].remaining_time = p[i].cpu_burst;
        p[i].ready_completed = FALSE;
        
        if (p[i].cpu_burst >= 2) {
            p[i].io_burst = (rand() % 3) + 1; // 1~3 범위의 랜덤한 수
            p[i].io_remaining_time = p[i].io_burst;
            p[i].io_start_time = (rand() % (p[i].cpu_burst - 1)) + 1; // 1~(cpu_burst-1) 범위의 랜덤한 수
        } else {
            p[i].io_burst = 0;
            p[i].io_remaining_time = 0;
            p[i].io_start_time = 0;
        }
    }
}

// process waiting_time, turnaround_time, remaining_time, ready_completed, io_remaining_time 초기화
void process_init(Process p[], int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].remaining_time = p[i].cpu_burst;
        p[i].ready_completed = FALSE;
        
        p[i].io_remaining_time = p[i].io_burst;
    }
}

void printGanttChart(Process p[], GanttQueue *ganttQueue) {
    // 큐에 총 몇 개의 값이 들어있는지 확인
    size_t queueSize = getQueueSize(ganttQueue);
    printf("Gantt Chart:\n");

    Node *current = ganttQueue->front;
    Node *previous = NULL;
    int startTime = 0;
    int ganttTime = 0;
    
    // ---- 표시
    while (current != NULL) {
        if (previous == NULL || current->data != previous->data) {
            if (previous != NULL) {
                printf("+------");
            }
            startTime = ganttTime;
        }
        previous = current;
        current = current->next;
        ganttTime++;
    }
    printf("+\n");
    
    // 프로세스 표시
    current = ganttQueue->front;
    previous = NULL;
    startTime = 0;
    ganttTime = 0;
    while (current != NULL) {
        if (previous == NULL || current->data != previous->data) {
            if (previous != NULL) {
                if (previous->data == -1) {
                    printf("| IDLE ");
                } else {
                    printf("|  P%-2d ", p[previous->data].pid);
                }
            }
            startTime = ganttTime;
        }
        previous = current;
        current = current->next;
        ganttTime++;
    }
    printf("|\n");
    
    // ---- 표시
    current = ganttQueue->front;
    previous = NULL;
    startTime = 0;
    ganttTime = 0;
    while (current != NULL) {
        if (previous == NULL || current->data != previous->data) {
            if (previous != NULL) {
                printf("+------");
            }
            startTime = ganttTime;
        }
        previous = current;
        current = current->next;
        ganttTime++;
    }
    printf("+\n");

    // 시간 표시
    current = ganttQueue->front;
    previous = NULL;
    ganttTime = 0;
    while (current != NULL) {
        if (previous == NULL || current->data != previous->data) {
            printf("%-2d     ", ganttTime);
        }
        previous = current;
        current = current->next;
        ganttTime++;
    }
    printf("\n");
}


int main() {
    int process_count;
    int quantum;

    printf("Enter the number of processes: ");
    scanf("%d", &process_count);

    Process *process = (Process *)malloc(sizeof(Process) * process_count);
    Create_Process(process, process_count);
    
    printf("Enter the time quantum for Round Robin scheduling: ");
    scanf("%d", &quantum);

    FCFS(process, process_count);
    SJF(process, process_count);
    Priority(process, process_count);
    Preem_SJF(process, process_count);
    Preem_Priority(process, process_count);
    RoundRobin(process, process_count, quantum);
    RoundRobin_priority(process, process_count, quantum);
    
    free(process);
    
    return 0;
}

void print_table(Process p[], int n) {
    puts("\t+-----+----------+------------+--------+-----------+-----------+------------+---------------+");
    puts("\t| PID |Burst Time|Arrival Time|Priority| I/O Burst | I/O start |Waiting Time|Turnaround Time|");
    puts("\t+-----+----------+------------+--------+-----------+-----------+------------+---------------+");
    for (int i = 0; i < n; i++) {
        printf("\t| %3d |   %3d    |    %3d     |  %3d   |    %3d    |    %3d    |     %3d    |      %3d      |\n",
            p[i].pid, p[i].cpu_burst, p[i].arrive_time, p[i].priority, p[i].io_burst, p[i].io_start_time, p[i].waiting_time, p[i].turnaround_time);
        puts("\t+-----+----------+------------+--------+-----------+-----------+------------+---------------+");
    }
}

void evaluate(Process p[], int n) {
    float total_waiting = 0, total_turnaround = 0;
    for (int i = 0; i < n; i++) {
        total_waiting += p[i].waiting_time;
        total_turnaround += p[i].turnaround_time;
    }
    float avg_waiting = total_waiting / n;
    float avg_turnaround = total_turnaround / n;
    printf("Average Waiting Time: %.3f\n", avg_waiting);
    printf("Average Turnaround Time: %.3f\n", avg_turnaround);
    puts("");
}

void FCFS(Process *p, int len) {
    printf("\n");
    printf("FCFS Scheduling\n");
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    // Sort by arrival time
    for (int i = 0; i < len - 1; i++) {
        for (int j = 0; j < len - i - 1; j++) {
            if (p[j].arrive_time > p[j + 1].arrive_time) {
                Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스를 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            running_index = dequeue(&readyQueue); // 프로세스 인덱스를 큐에서 추출
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}

void SJF(Process *p, int len) {
    printf("Non-preemptive SJF Scheduling\n");
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            int shortest_index = readyQueue.front->data;
            
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // 남은 cpu burst time 가장 짧은 것 찾기
            while (temp != NULL) {
                if (p[temp->data].remaining_time < p[shortest_index].remaining_time) {
                    shortest_index = temp->data;
                }
                temp = temp->next;
            }
            running_index = shortest_index;
            
            // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
            if (running_index == readyQueue.front->data) {
                dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
            } else {
                temp = readyQueue.front;
                while (temp->next != NULL && temp->data != running_index) {
                    prev_temp = temp;
                    temp = temp->next;
                }
                // running_index 위치 찾으면 이전 노드와 다음 노드 연결
                prev_temp->next = temp->next;
                if (prev_temp->next == NULL) {
                    readyQueue.rear = prev_temp;
                }
                // 이전 노드와 다음 노드 연결 시켰으면 deque
                free(temp);
            }
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}

// Priority 값이 낮을수록 우선순위가 높다
void Priority(Process *p, int len) {
    printf("Non-preemptive Priority Scheduling\n");
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            int high_priority_index = readyQueue.front->data;
            
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // 우선순위 높은 process 찾기
            while (temp != NULL) {
                if (p[temp->data].priority < p[high_priority_index].priority) {
                    high_priority_index = temp->data;
                }
                temp = temp->next;
            }
            running_index = high_priority_index;
            
            // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
            if (running_index == readyQueue.front->data) {
                dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
            } else {
                temp = readyQueue.front;
                while (temp->next != NULL && temp->data != running_index) {
                    prev_temp = temp;
                    temp = temp->next;
                }
                prev_temp->next = temp->next;
                if (prev_temp->next == NULL) {
                    readyQueue.rear = prev_temp;
                }
                free(temp);
            }
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}

void Preem_SJF(Process *p, int len) {
    printf("Preemptive SJF Scheduling\n");
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // remaining time이 더 짧은 process가 나타났을 때
        if (running_index != -1 && !isEmpty(&readyQueue)) {
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            int shortest_index = readyQueue.front->data;
            while (temp != NULL) {
                if (p[temp->data].remaining_time < p[shortest_index].remaining_time) {
                        shortest_index = temp->data;
                }
                temp = temp->next;
            }
            if (p[running_index].remaining_time > p[shortest_index].remaining_time) {
                enqueue(&readyQueue, running_index);
                running_index = shortest_index;
                
                if (running_index == readyQueue.front->data) {
                    dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
                } else {
                    temp = readyQueue.front;
                    while (temp->next != NULL && temp->data != running_index) {
                        prev_temp = temp;
                        temp = temp->next;
                    }
                    prev_temp->next = temp->next;
                    if (prev_temp->next == NULL) {
                        readyQueue.rear = prev_temp;
                    }
                    free(temp);
                }
                p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
            }
        }
            
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            int shortest_index = readyQueue.front->data;
            
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // 제일 작은 remaining time 찾기기
            while (temp != NULL) {
                if (p[temp->data].remaining_time < p[shortest_index].remaining_time) {
                    shortest_index = temp->data;
                }
                temp = temp->next;
            }
            running_index = shortest_index;
            
            // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
            if (running_index == readyQueue.front->data) {
                dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
            } else {
                temp = readyQueue.front;
                while (temp->next != NULL && temp->data != running_index) {
                    prev_temp = temp;
                    temp = temp->next;
                }
                prev_temp->next = temp->next;
                if (prev_temp->next == NULL) {
                    readyQueue.rear = prev_temp;
                }
                free(temp);
            }
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
            
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}

void Preem_Priority(Process *p, int len) {
    printf("Preemptive Priority Scheduling\n");
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 우선순위가 더 높은 process가 나타났을 때
        if (running_index != -1 && !isEmpty(&readyQueue)) {
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            int high_priority_index = readyQueue.front->data;
            while (temp != NULL) {
                if (p[temp->data].priority < p[high_priority_index].priority) {
                        high_priority_index = temp->data;
                }
                temp = temp->next;
            }
            if (p[running_index].priority > p[high_priority_index].priority) {
                enqueue(&readyQueue, running_index);
                running_index = high_priority_index;
                
                if (running_index == readyQueue.front->data) {
                    dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
                } else {
                    temp = readyQueue.front;
                    while (temp->next != NULL && temp->data != running_index) {
                        prev_temp = temp;
                        temp = temp->next;
                    }
                    prev_temp->next = temp->next;
                    if (prev_temp->next == NULL) {
                        readyQueue.rear = prev_temp;
                    }
                    free(temp);
                }
                p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
            }
        }
            
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            int high_priority_index = readyQueue.front->data;
            
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // 우선순위 높은 process 찾기
            while (temp != NULL) {
                if (p[temp->data].priority < p[high_priority_index].priority) {
                    high_priority_index = temp->data;
                }
                temp = temp->next;
            }
            running_index = high_priority_index;
            
            // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
            if (running_index == readyQueue.front->data) {
                dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
            } else {
                temp = readyQueue.front;
                while (temp->next != NULL && temp->data != running_index) {
                    prev_temp = temp;
                    temp = temp->next;
                }
                prev_temp->next = temp->next;
                if (prev_temp->next == NULL) {
                    readyQueue.rear = prev_temp;
                }
                free(temp);
            }
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
            
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}

void RoundRobin(Process *p, int len, int quantum) {
    printf("Round Robin Scheduling with Quantum %d\n", quantum);
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    int remaining_quantum = quantum; // 남은 quantum 시간
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            remaining_quantum--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 실행 중인 프로세스가 끝나지 않았지만, quantum 시간을 다 사용했을 때
        if (running_index != -1 && remaining_quantum == 0) {
            enqueue(&readyQueue, running_index);
            running_index = -1;
        }
            
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            remaining_quantum = quantum;
            running_index = dequeue(&readyQueue); // 프로세스 인덱스를 큐에서 추출
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }   
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}


// 추가 구현 : RR + Priority
void RoundRobin_priority(Process *p, int len, int quantum) {
    printf("Round Robin + Priority Scheduling with Quantum %d\n", quantum);
    int current_time = 0;
    int completed_processes = 0;
    int running_index = -1; // 현재 실행 중인 프로세스 index
    process_init(p, len);
    int remaining_quantum = quantum; // 남은 quantum 시간
    
    Queue readyQueue;
    initQueue(&readyQueue);
    
    Queue waitingQueue;
    initQueue(&waitingQueue);
    
    Queue ganttQueue;
    initQueue(&ganttQueue);
    
    while (completed_processes != len) {
        // ready queue에 도착한 프로세스 추가
        for (int i = 0; i < len; i++) {
            if (current_time >= p[i].arrive_time && !p[i].ready_completed) {
                enqueue(&readyQueue, i); // 프로세스 인덱스를 큐에 삽입
                p[i].ready_completed = TRUE;
            }
        }
        
        // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
        if (!isEmpty(&waitingQueue)) {
            int wait_index = waitingQueue.front->data;
            
            Node *temp = waitingQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // waitingQueue 안의 process들의 io_remaining_time 값 하나씩 감소
            while (temp != NULL) {
                p[temp->data].io_remaining_time--;
                // io_remaining_time이 0이 되면 deque
                if (p[temp->data].io_remaining_time==0) {
                    wait_index = temp->data;
                    
                    Node *next_node = temp->next; // 다음 노드를 미리 저장
                    
                    // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
                    if (wait_index == waitingQueue.front->data) {
                        dequeue(&waitingQueue); // 맨 앞 노드를 삭제하면서 반환
                    } else {
                        temp = waitingQueue.front;
                        // wait_index 노드 찾기
                        while (temp->next != NULL && temp->data != wait_index) {
                            prev_temp = temp;
                            temp = temp->next;
                        }
                        // wait_index 위치 찾으면 이전 노드와 다음 노드 연결 
                        prev_temp->next = temp->next;
                        if (prev_temp->next == NULL) {
                            waitingQueue.rear = prev_temp;
                        }
                        // 이전 노드와 다음 노드 연결 시켰으면 deque
                        free(temp);
                    }
                    // readyQueue에 해당 프로세스를 추가
                    enqueue(&readyQueue, wait_index);
                    temp = next_node; // 다음 노드로 이동
                    
                } else {
                    temp = temp->next;
                }
            }
        }
        
        // 실행 중인 프로세스 안 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time != 0) {
            p[running_index].remaining_time--;
            remaining_quantum--;
            // cpu_burst - remaining time = io_start_time이고, i/o_time이 존재하면 i/o start
            if (p[running_index].io_start_time == p[running_index].cpu_burst-p[running_index].remaining_time && p[running_index].io_remaining_time>0) {
                enqueue(&waitingQueue, running_index);
                running_index = -1;
            }
        }
        
        // 실행 중인 프로세스 끝났을 때
        if (running_index != -1 && p[running_index].remaining_time == 0) {
            completed_processes++;
            p[running_index].turnaround_time = current_time - p[running_index].arrive_time;
            running_index = -1;
        }
        
        // 실행 중인 프로세스가 끝나지 않았지만, quantum 시간을 다 사용했을 때
        if (running_index != -1 && remaining_quantum == 0) {
            enqueue(&readyQueue, running_index);
            running_index = -1;
        }

        // 우선순위가 더 높은 process가 나타났을 때
        if (running_index != -1 && !isEmpty(&readyQueue)) {
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            int high_priority_index = readyQueue.front->data;
            while (temp != NULL) {
                if (p[temp->data].priority < p[high_priority_index].priority) {
                        high_priority_index = temp->data;
                }
                temp = temp->next;
            }
            if (p[running_index].priority > p[high_priority_index].priority) {
                enqueue(&readyQueue, running_index);
                running_index = high_priority_index;
                
                if (running_index == readyQueue.front->data) {
                    dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
                } else {
                    temp = readyQueue.front;
                    while (temp->next != NULL && temp->data != running_index) {
                        prev_temp = temp;
                        temp = temp->next;
                    }
                    prev_temp->next = temp->next;
                    if (prev_temp->next == NULL) {
                        readyQueue.rear = prev_temp;
                    }
                    free(temp);
                }
                remaining_quantum = quantum;
                p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
            }
        }
            
        // 실행 중인 프로세스가 없으면 ready queue에서 프로세스 꺼내 실행
        if (running_index == -1 && !isEmpty(&readyQueue)) {
            int high_priority_index = readyQueue.front->data;
            
            Node *temp = readyQueue.front;
            Node *prev_temp = NULL; // 이전 노드를 기억하기 위한 포인터
            
            // 우선순위 높은 process 찾기
            while (temp != NULL) {
                if (p[temp->data].priority < p[high_priority_index].priority) {
                    high_priority_index = temp->data;
                }
                temp = temp->next;
            }
            running_index = high_priority_index;
            
            // 노드를 꺼낼 때, 맨 앞 노드가 아니면 꺼내는 노드의 이전 노드와 연결 시키기
            if (running_index == readyQueue.front->data) {
                dequeue(&readyQueue); // 맨 앞 노드를 삭제하면서 반환
            } else {
                temp = readyQueue.front;
                while (temp->next != NULL && temp->data != running_index) {
                    prev_temp = temp;
                    temp = temp->next;
                }
                prev_temp->next = temp->next;
                if (prev_temp->next == NULL) {
                    readyQueue.rear = prev_temp;
                }
                free(temp);
            }
            remaining_quantum = quantum;
            p[running_index].waiting_time = current_time - p[running_index].arrive_time - p[running_index].cpu_burst + p[running_index].remaining_time - p[running_index].io_burst;
        }
        current_time++;
        enqueue(&ganttQueue, running_index);
    }
    printGanttChart(p, &ganttQueue);
    print_table(p, len);
    evaluate(p, len);
}
