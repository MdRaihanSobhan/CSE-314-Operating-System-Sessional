#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>

#define ll long long int 

#define NOT_ARRIVED 0 // not arrived at printer
#define WAITING 1 // waiting for printer
#define PRINTING 2 // printing
#define DONE 3 // done printing

#define FREE 0 // free printer
#define BUSY 1 // busy printer


#define MAX_PRINTER 4           // number of printers
#define BINDERS 2               // number of binders
#define MAX_STUDENT 10000       // number of maximum students
#define MAX_GROUP 10000         // number of maximum groups
#define STAFF 2                // number of staff
using namespace std;    

pthread_mutex_t lock;          // print lock
// pthread_mutex_t printerlock;    // printer lock

pthread_t student_thread[MAX_STUDENT]; // student threads
pthread_t staff_thread[STAFF]; // staff threads


sem_t binders;  // binders semaphore
sem_t rc_lock;  // reader count lock
sem_t db_lock;  // database lock to variable submission_count
sem_t students[MAX_STUDENT]; // student semaphores
sem_t printers[MAX_PRINTER]; // printer semaphores

time_t start_time; // start time
time_t end_time; // end time

ll N; // number of students 
ll M; // size of each group
ll w; // Printing time
ll x; // binding time
ll y; // reading writing time
ll student_delay[MAX_STUDENT]; // student delays for arrival
ll staff_delay[STAFF];          // staff random delays for reading period
ll student_state[MAX_STUDENT];  // student state
ll printer_state[MAX_PRINTER];  // printer state
ll group_print_count[MAX_GROUP]; // number of students printed in a group
ll rc = 0; // reader count
ll submission_count = 0; // number of submissions

// get current time

ll getcurrenttime(){
    time(&end_time); 
    double time = double(end_time - start_time);
    return (ll) time; 
}

// generate random delay for students' arrival
ll generateRandomDelay(double lambda) {
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::poisson_distribution<ll> distribution(lambda);
    return distribution(generator);
}

// check if printer is free
void test(ll i){
    if(student_state[i] == WAITING && printer_state[(i+1)%MAX_PRINTER] == FREE){
        printer_state[(i+1)%MAX_PRINTER] = BUSY;
        student_state[i] = PRINTING;
        sem_post(&students[i]);
    }
}

// print assignment
void printing(ll id){
    sem_wait(&printers[(id+1)%MAX_PRINTER]);    // wait for printer
    test(id); // check if printer is free
    sem_post(&printers[(id+1)%MAX_PRINTER]);   // release printer
    sem_wait(&students[id]);                    // wait for printing
    sem_wait(&printers[(id+1)%MAX_PRINTER]);    // wait for printer
    pthread_mutex_lock(&lock);                  
    printf("Student %lld has started printing at printer %lld at time %lld\n", id+1, (id+1)%MAX_PRINTER +1, getcurrenttime());
    pthread_mutex_unlock(&lock);
    sleep(w);    
    pthread_mutex_lock(&lock);
    printf("Student %lld has finished printing at printer %lld at time %lld\n", id+1, (id+1)%MAX_PRINTER +1, getcurrenttime());
    pthread_mutex_unlock(&lock);

    student_state[id] = DONE; // student has finished printing
    printer_state[(id+1)%MAX_PRINTER] = FREE; // printer is free

    ll group_no = id/M; // group number
    group_print_count[group_no]++; // increment the number of students printed in the group

    bool flag = false;
    // check if any student in my group is waiting for printer
    for(ll i=group_no*M; i<(group_no+1)*M; i++){
        if(student_state[i] == WAITING && (i+1)%MAX_PRINTER == (id+1)%MAX_PRINTER){
            flag = true; // found a student in my group waiting for printer
            test(i); 
            break; // only one student can print at a time
        }
    }
    // if no student in my group is waiting for printer, check if any other student is waiting for printer
    if(flag == false){
        for(ll i=0; i<N; i++){
            if(student_state[i] == WAITING && (i+1)%MAX_PRINTER == (id+1)%MAX_PRINTER){
                test(i); // found a student waiting for printer
                break; // only one student can print at a time
            }
        }
    }
    sem_post(&printers[(id+1)%MAX_PRINTER]); // release printer
    

}

void bind(ll id){
    sem_wait(&binders); // wait for binder 
    pthread_mutex_lock(&lock); 
    printf("\nGroup %lld has started binding at time %lld\n", id/M+1, getcurrenttime());
    pthread_mutex_unlock(&lock);
    sleep(x); // binding time
    pthread_mutex_lock(&lock);
    printf("\nGroup %lld has finished binding at time %lld\n", id/M+1, getcurrenttime());
    pthread_mutex_unlock(&lock);
    sem_post(&binders);     // release binder
}

void writer(ll groupid){
    
    sem_wait(&db_lock); // wait for database lock for submission_count
    submission_count++; 

    pthread_mutex_lock(&lock);
    printf("Group %lld has arrived to submit the report at time %lld\n", groupid+1, getcurrenttime());
    pthread_mutex_unlock(&lock);
     
    sleep(y); // writing time

    pthread_mutex_lock(&lock);
    printf("Group %lld has submitted the report at time %lld\n", groupid+1, getcurrenttime());
    pthread_mutex_unlock(&lock);
    sem_post(&db_lock); // release database lock for submission_count
}

ll terminated[MAX_STUDENT]; // terminated students

void * student(void* arg)
{	
    ll id = (ll)arg; 
    sleep(student_delay[id]); //  delay before arrival
    pthread_mutex_lock(&lock);
    printf("Student %lld has arrived at the print station at time %lld \n", id+1, getcurrenttime());
    student_state[id] = WAITING;
    pthread_mutex_unlock(&lock);

    printing(id); // print assignment
    
    // group leader waits for all students in the group to finish printing
    if((id+1)%M==0){
        for(ll i=(id/M)*M; i<(id/M+1)*M; i++){
            if(i==id || terminated[i]==1){
                continue;
            }
            else{
                pthread_join(student_thread[i],NULL);
            }
        }
    }
    else{
        terminated[id] =1; 
        return NULL; // other students except group leader exit
    }

    pthread_mutex_lock(&lock);
    printf("\nGroup %lld has finished printing at time %lld\n \n", id/M+1, getcurrenttime());
    pthread_mutex_unlock(&lock);
    
    // group leader binds the assignment
    bind(id);

    // group leader submits the assignment
    writer(id/M);

    pthread_exit(NULL);
}

void* reader(void* arg){
    ll staffno = (ll)arg;   // staff number
    while(true){
        sleep(staff_delay[staffno]); // delay period of staff before reading each time
        sem_wait(&rc_lock);
        rc++;
        if(rc == 1){
            sem_wait(&db_lock);
        }
        sem_post(&rc_lock);


        // read
        pthread_mutex_lock(&lock);
        printf("Staff %lld has started reading the entry book at time %lld. No of submission = %lld \n", staffno+1, getcurrenttime(), submission_count);
        pthread_mutex_unlock(&lock);
        sleep(y); // reading time
        pthread_mutex_lock(&lock);
        printf("Staff %lld has finished reading the entry book at time %lld. No of submission = %lld \n", staffno+1, getcurrenttime(), submission_count);
        pthread_mutex_unlock(&lock);
        if(submission_count == N/M){
            return NULL; // all submissions are done
        }
        sem_wait(&rc_lock);
        rc--;
        if(rc == 0){
            sem_post(&db_lock);
        }
        sem_post(&rc_lock);
    }
}



void initialize(){

}

int main(void)
{	
    printf("Number of Students: ");
    scanf("%lld", &N);
    printf("Number of Students in a group: ");    
    scanf("%lld", &M);
    printf("Printing time: ");
    scanf("%lld", &w);
    printf("Binding time: ");
    scanf("%lld", &x);
    printf("Reading writing time: ");
    scanf("%lld", &y);
    
    pthread_mutex_init(&lock,0);

    sem_init(&binders,0,BINDERS);
    sem_init(&rc_lock,0,1);
    sem_init(&db_lock,0,1);

    for(ll i=0;i<N;i++){
        sem_init(&students[i],0,1);
        sem_wait(&students[i]);
        terminated[i] = 0;
    }
    for(ll i=0; i<MAX_PRINTER; i++){
        sem_init(&printers[i],0,1);
    }

    for(ll i=0;i<N/M; i++){
        group_print_count[i] = 0;
    }

    for(ll i=0;i<N;i++){
        student_delay[i] = 3+ generateRandomDelay(7.0)%20;
        student_state[i] = NOT_ARRIVED;
    }
    for(ll i=0; i<MAX_PRINTER; i++){
        printer_state[i] = FREE;
    }
    for(ll i=0; i<STAFF; i++){
        staff_delay[i] = 1+ generateRandomDelay(10.0)%10;
    }
    time(&start_time);
    for(ll i=0;i<N;i++){
        pthread_create(&student_thread[i],NULL,student, (void*)i);  
    }

    for(ll i=0; i<STAFF; i++){
        pthread_create(&staff_thread[i],NULL,reader, (void*)i);  
    }

    for(ll i=N/M -1;i<N;i+=M){
        pthread_join(student_thread[i],NULL); 
    }


    for(ll i=0; i<STAFF ; i++){
        pthread_join(staff_thread[i], NULL);
    }

    // while(1);

	return 0;
}
