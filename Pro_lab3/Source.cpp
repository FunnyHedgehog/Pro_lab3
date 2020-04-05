#include <mpi.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <random>

using namespace std;

MPI_Status status;
MPI_Request request;

vector<int> randomVector(size_t size)
{
	vector<int> v(size);
	generate(v.begin(), v.end(), []() { return rand() % 100 + 1; });
	return v;
}
void printRecievedMessages(vector<int> messages, int rank) {
	for (auto i : messages)
	{
		cout << "From rank #" << rank << ": The received message: " << i << endl;
	}
}
void exchangeDataBetweenAllProcesses(vector<int> messages) {
	for (auto i : messages) {
		MPI_Bcast(&i, 1, MPI_INT, i, MPI_COMM_WORLD);
	}
}
vector<int> initMessages(int size) {
	vector<int>messages;
	for (int i = 0; i < size; i++) {
		messages.push_back(i);
	}
	return messages;
}
void firstTask(int rank,int size) {
	vector<int>messages = initMessages(size);
	exchangeDataBetweenAllProcesses(messages);
	if (rank % 2 == 0 || rank % 3 == 0)//парні та кратні 3
	{
		printRecievedMessages(messages, rank);
	}
}

void bubbleSort(int *a, int n)
{
	for (int i = n - 1; i >= 0; i--)
		for (int j = 0; j < i; j++)
		{
			if (a[j] > a[j + 1])
				swap(a[j], a[j + 1]);
		}
}
void updateBuffer(int* buffer, int* returnedArray) {
	for (int i = 4; i < 8; i++)
	{
		buffer[i] = returnedArray[i - 4];
	}
}
void updateReturnedArray(int* buffer, int* returnedArray) {
	for (int i = 0; i < 4; i++)
	{
		returnedArray[i] = buffer[i];
	}
}
void secondTask(int rank, int p1, int p2, int* returnedArray)
{
	if (rank == p1)
	{
		int buffer[8];
		MPI_Irecv(buffer, 4, MPI_INT, p2, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		updateBuffer(buffer, returnedArray);
		bubbleSort(buffer, 8);
		updateReturnedArray(buffer, returnedArray);
		MPI_Isend(buffer + 4, 4, MPI_INT, p2, 0, MPI_COMM_WORLD, &request);
	} 
	else if (rank == p2)
	{
		MPI_Isend(returnedArray, 4, MPI_INT, p1, 0, MPI_COMM_WORLD, &request);
		MPI_Irecv(returnedArray, 4, MPI_INT, p1, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
	}
}
void printArray(vector<int> numberArray) {
	for (auto i : numberArray) {
		cout << i << " ";
	}
	cout << endl;
}
int main(int argc, char *argv[])
{
	int rank, size, root = 0;
	vector<int> numberArray(12);
	vector<int> returnedArray(4);

	srand(time(NULL));
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	firstTask(rank,size);
	//secondTask
	if (size == 3)
	{
		if (rank == 0)
		{
			numberArray = randomVector(12);
			cout << "\nFrom rank #" << rank << ": Not sorted array: ";
			printArray(numberArray);
		}
		MPI_Scatter(numberArray.data(), 4, MPI_INT, returnedArray.data(), 4, MPI_INT, root, MPI_COMM_WORLD);
		secondTask(rank, 0, 1, returnedArray.data());
		secondTask(rank, 0, 2, returnedArray.data());
		secondTask(rank, 1, 2, returnedArray.data());
		MPI_Gather(returnedArray.data(), 4, MPI_INT, numberArray.data(), 4, MPI_INT, root, MPI_COMM_WORLD);

		if (rank == 0)
		{
			cout << "From rank #" << rank << ": Sorted array:     ";
			printArray(numberArray);
		}
	}
	else
		if (rank == 0)
			cout << "From rank #" << rank << ": You need to create 3 processes to enable bubble sort"<<endl;

	MPI_Finalize();
	return 0;
}