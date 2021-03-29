/****************************************************/
/*	AUTHORS: CABARRON, Oscar III L.					*/
/*			 FEROLIN, Mary Bernadette J.			*/
/*			 TORRES, Ariel Joy C.					*/
/*													*/
/*	DATE FINISHED: December 20, 2020				*/
/*													*/
/*	GROUP NUMBER: 6									*/
/*	TEAM NUMBER: 9									*/
/*													*/
/*	COURSE CODE: CS 3104 - OPERATING SYSTEMS		*/
/*	COURSE INSTRUCTOR: Ms. Blasminda Catubig Mayol	*/
/*	COURSE SCHEDULE: 4:30 - 6:00, MW				*/
/*													*/
/*	PROGRAM DESCRIPTION:							*/
/*	The program is a CPU process scheduling program	*/
/*	that accepts a number of processes and their 	*/
/*	corresponding Arrival Times, Priority and Burst */
/*	times. The algorithm is a Preemptive Priority	*/
/*	Scheduling Algorithm. The outputs of the program*/
/*	are the table of processes, the average waiting	*/
/*	and turnaround times, as well as the Gantt Chart*/
/*	which illustrates how the process is scheduled	*/
/*	to use the CPU. 								*/
/****************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<conio.h>

typedef struct {
	int arrivalTime;
	int burstTimeCPU;
	int priority;
	int PID;
	int completionTime;
	int turnaroundTime;
	int waitingTime;
}Process;

typedef struct {
	Process* processArray;
	int sizeProcesses;
	int count;
}ArrayContainer;

void askInputArray(ArrayContainer*);
ArrayContainer initializeQueue(ArrayContainer);
ArrayContainer ganttChart(ArrayContainer);
void displayProcesses(ArrayContainer);
void executeCPU(ArrayContainer* , ArrayContainer* );
void sortArrivalTime(ArrayContainer* );
void displayGantt(ArrayContainer, ArrayContainer* );
void computeTurnaroundTime(ArrayContainer* );
void computeWaitingTime(ArrayContainer* );
void computeCompletionTime(ArrayContainer, ArrayContainer* );
void prompt();

int main (void)
{
	/*Variable Delcaration*/
	ArrayContainer proc, gantt;
	
	/*Variable Definition*/
	askInputArray(&proc);
	gantt = ganttChart(proc);
	
	/*Function calls and others*/
	sortArrivalTime(&proc);
	executeCPU(&proc, &gantt);
	prompt(); //prompts the user to continue (for neatness of display)
	computeCompletionTime(gantt, &proc);
	computeTurnaroundTime(&proc);
	computeWaitingTime(&proc);
	prompt();
	displayProcesses(proc);
	prompt();
	displayGantt(gantt, &proc);
		
	return 0;
}

void askInputArray(ArrayContainer* A)
{
	int i, numP;
	printf("Note:");
	printf("\n1. The HIGHEST priority is 0.");
	printf("\n2. The FIRST arrival time is 0.0 milliseconds.");
	printf("\n3. You cannot have a CPU burst of 0.0 milliseconds.");
	do{
		printf("\n\nEnter the number of processes: ");
		scanf("%d", &numP);
	}while(numP <= 0);
	
	/*Initializes the Array Container of the processes*/
	A->processArray = (Process*)calloc(numP, sizeof(Process));
	A->sizeProcesses = numP;
	A->count = 0;
	
	printf("\n");
	if(A->processArray!=NULL){
		for(i=0; i<A->sizeProcesses; i++){
			A->processArray[i].PID = i+1;
			do{
				printf("Arrival Time of process P%d: ", A->processArray[i].PID);
				scanf("%d", &A->processArray[i].arrivalTime);	
			}while(A->processArray[i].arrivalTime < 0);
			do{
				printf("CPU Burst Time of process P%d: ", A->processArray[i].PID);
				scanf("%d", &A->processArray[i].burstTimeCPU);
			}while(A->processArray[i].burstTimeCPU <= 0);
			do{
				printf("Priority number of process P%d: ", A->processArray[i].PID);
				scanf("%d", &A->processArray[i].priority);
			}while(A->processArray[i].priority < 0);
			printf("\n");
			++A->count;
		}
	}
}

void displayProcesses(ArrayContainer A)
{
	int i;
	printf("\nTable of Processes");
	printf("\n%-10s%-20s%-20s%-15s%-15s%-15s%-15s", 
				"PID", 
				"Arrival", 
				"Burst", 
				"Priority", 
				"Completion", 
				"Turnaround", 
				"Waiting");
	printf("\n");
	for(i=0; i<A.count; i++){
		printf("%-10d", A.processArray[i].PID);
		printf("%-20.2f", (float)A.processArray[i].arrivalTime);
		printf("%-20.2f", (float)A.processArray[i].burstTimeCPU);
		printf("%-15d", A.processArray[i].priority);
		printf("%-15.2f", (float)A.processArray[i].completionTime);
		printf("%-15.2f", (float)A.processArray[i].turnaroundTime);
		printf("%-15.2f", (float)A.processArray[i].waitingTime);
		printf("\n");
	}
}

ArrayContainer initializeQueue(ArrayContainer A)
{
	ArrayContainer Q;
	Q.processArray = (Process*) calloc (A.processArray[A.count-1].arrivalTime, sizeof(Process));
	Q.sizeProcesses = A.processArray[A.count-1].arrivalTime;
	Q.count = 0;
	return Q;
}

ArrayContainer ganttChart(ArrayContainer A)
{
	ArrayContainer gantt;
	int i;
	
	gantt.count = 0;
	gantt.sizeProcesses = 0;
	
	/*maximum size of the Gantt Chart*/
	for(i=0; i<A.count; i++){
		gantt.sizeProcesses+=A.processArray[i].burstTimeCPU;
	}
	
	/*This accommodates for when the arrival time is not 0*/
	gantt.sizeProcesses+=A.processArray[0].arrivalTime;
	
	/*Creation of the actual Gantt Chart*/
	gantt.processArray = (Process*) calloc ((gantt.sizeProcesses), sizeof(Process));
	
	return gantt;
}

void sortArrivalTime(ArrayContainer* processes)
{
	/*
		The processes are sorted according to their arrival.
		The sorting algorithm used is Insertion Sort
	*/
	int i, j;
	Process mark;
	for(i=0; i<processes->count; i++){
		mark = processes->processArray[i];
		for(j=i-1; j >= 0 && processes->processArray[j].arrivalTime > mark.arrivalTime; j--){
			processes->processArray[j+1] = processes->processArray[j];
		}
		processes->processArray[j+1] = mark;
	}	
}

void executeCPU(ArrayContainer* proc, ArrayContainer* gantt)
{
	ArrayContainer readyQueue;
	readyQueue = initializeQueue(*proc);
	int clock, i, j=0, priority, first;
	
	for(clock = 0; clock < gantt->sizeProcesses; clock++){
		/*Stores the process whose arrival time is the same as the clock tick*/
		for(i=0; i<proc->count; i++){
			if(proc->processArray[i].arrivalTime == clock){
				readyQueue.processArray[j] = proc->processArray[i];
				++readyQueue.count;
				j++;
			}
		}
			
		if(readyQueue.count > 0){ //If the queue is not empty
			if(readyQueue.count == 1){
				first = 0;
			}else{
				/*
					If there are more than one processes,
					at the particular time, it compares the priority.
					The process whose priority has the greatest value
					(least number) is accommodated to run first.
					This is especially helpful when two processes arrive
					at the same time. 
				*/
				for(i=0, priority = INT_MAX; i<readyQueue.count; i++){
					if(readyQueue.processArray[i].priority < priority){
						priority = readyQueue.processArray[i].priority;
						first = i;
					}
				}
			}
				
			/*The process is now placed in the Gantt Chart*/	
			gantt->processArray[clock] = readyQueue.processArray[first];
			++gantt->count;
				
			/*Decrements 1 millisecond from the burst time*/
			readyQueue.processArray[first].burstTimeCPU--;
			
			/*
				Once the process has a burst time of 0,
				it is removed from the readyQueue by shifting
				the elements of the readyQueue to the left
			*/
			if(readyQueue.processArray[first].burstTimeCPU == 0){
				for(i=first;i<readyQueue.count;i++){
					readyQueue.processArray[i]=readyQueue.processArray[i+1];
				}
				readyQueue.count--;
				j--;
			}
		}
	}
}

void displayGantt(ArrayContainer gantt, ArrayContainer* proc)
{
	int i, previous;
	printf("\nCPU Execution Gantt Chart\n");
	for(i=0; i<gantt.sizeProcesses; i++){
		printf("------");
	}
	printf("\n");
	for(i=0, previous = -1; i<gantt.sizeProcesses; i++){
		/*Displaying the first process in the Gantt chart*/
		if(previous == -1){
			printf("|%d: P%d ",i,gantt.processArray[0].PID);
		/*
			If the current process is still the same as the previous one.
			If true, it just prints a space which is the portion
			of the chart allocated to that process.
		*/
		}else if (gantt.processArray[previous].PID == gantt.processArray[i].PID){
			printf("    ");
		}else{ //Another process has taken the CPU control
			printf("|%d: P%d ",i,gantt.processArray[i].PID);
		}
		previous = i;
	}
	printf("|%d",i);
	printf("\n");
	for(i=0; i<gantt.sizeProcesses; i++){
		printf("------");
	}
}

void computeTurnaroundTime(ArrayContainer* proc)
{
	/*The average turnaround time is the average of all the process's turnaround times*/
	/*Turnaround Time = Completion Time - Arrival Time*/
	int i;
	float avgTurnaroundTime = 0;
	
	/*Each process is calculated their individual turnaround time*/
	for(i=0; i<proc->sizeProcesses; i++){
		proc->processArray[i].turnaroundTime 
			= proc->processArray[i].completionTime - proc->processArray[i].arrivalTime;
		avgTurnaroundTime+=proc->processArray[i].turnaroundTime;
	}
	
	avgTurnaroundTime/=proc->sizeProcesses;
	printf("\nAverage Turnaround Time: %.2f milliseconds", avgTurnaroundTime);
}

void computeWaitingTime(ArrayContainer* proc)
{
	/*The average waiting time is the average of all the process's waiting times*/
	/*Waiting Time = Turnaround Time - Burst Time*/
	int i;
	float avgWaitingTime=0;
	
	/*Each process is calculated their individual waiting time*/
	for(i=0; i<proc->sizeProcesses; i++){
		proc->processArray[i].waitingTime = 
			proc->processArray[i].turnaroundTime - proc->processArray[i].burstTimeCPU;
			
		avgWaitingTime+=proc->processArray[i].waitingTime;
	}
	
	avgWaitingTime/=(float)proc->sizeProcesses;
	printf("\nAverage Waiting Time: %.2f milliseconds\n", avgWaitingTime);
}

void computeCompletionTime(ArrayContainer gantt, ArrayContainer* proc)
{
	int i, previous;
	
	/*Loops through the Gantt Chart*/
	for(i=0, previous = -1; i<gantt.sizeProcesses; i++){
		if(previous!=-1){
			
			/*
				Checks if one process has ended by checking 
				if the current PID is different than the previous
				one.
				If one process has ended, its completion time
				is stored in proc->processArray[gantt.processArray[previous].PID-1].completionTime
			*/
			if(gantt.processArray[previous].PID != gantt.processArray[i].PID){
				proc->processArray[gantt.processArray[previous].PID-1].completionTime = i;
			}
		}
		previous = i;
	}
	proc->processArray[proc->sizeProcesses-1].completionTime = i;
}

void prompt()
{
	printf("\n\nPress any key to continue...\n\n");
	getch();
}


