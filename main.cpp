#include <iostream>
#include <pthread.h>
#include <fstream>
#include <cstring>
#include <unistd.h>

using namespace std;

static int maxNCarsInTunnel = 0;
static int maxNNBCarsInTunnel = 0;
static int maxNSBCarsInTunnel = 0;
static int curNCarsInTunnel = 0;
static int curNNBCarsInTunnel = 0;
static int curNSBCarsInTunnel = 0;
static int totalNCarsInTunnel = 0;
static int totwaitNCars = 0;
static int totalNNBCars = 0;
static int totalNSBCars = 0;

static pthread_mutex_t car_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t traffc_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wake_up = PTHREAD_COND_INITIALIZER;

struct Car {
	int carNo, travelTime;
};

// function to get the special chacater separated value with the help of index
string entry(string str, int idx, char ch) {
	str += '\0';
	string value = "";
	int countSpace = 0;
	int i = 0;
	for (int i = 0; i < str.length(); i++) {
		if (idx == countSpace - 1) break;
		if (str[i] == ch) {
			countSpace++;
			if (idx == countSpace - 1) {
				break;
			} else {
				value = "";
			}
		} else value += str[i];

	}
	return value;
} // entry

void *carNB(void *arg) {
	struct Car *car = (struct Car *) arg;

	pthread_mutex_lock(&car_lock);

	cout << "Northnound car # " << car -> carNo << " arrives at the tunnel." << endl;

	if (!(curNCarsInTunnel < maxNCarsInTunnel && curNNBCarsInTunnel < maxNNBCarsInTunnel)) {
		totwaitNCars++;
		while (!(curNCarsInTunnel < maxNCarsInTunnel && curNNBCarsInTunnel < maxNNBCarsInTunnel)) {
			pthread_cond_wait(&wake_up, &car_lock);
		}
	}

	curNCarsInTunnel++;
	curNNBCarsInTunnel++;

	cout << "Northnound car # " << car -> carNo << " enters the tunnel." << endl;

	pthread_mutex_unlock(&car_lock);

	sleep(car -> travelTime);

	pthread_mutex_lock(&traffc_lock);
	totalNNBCars++;
	curNCarsInTunnel--;
	curNNBCarsInTunnel--;
	cout << "Northnound car # " << car -> carNo << " exits the tunnel." << endl;
	pthread_cond_signal(&wake_up);
	pthread_cond_broadcast(&wake_up);
	pthread_mutex_unlock(&traffc_lock);

	pthread_exit((void*) 0);
} // carNB

void *carSB(void *arg) {
	struct Car *car = (struct Car *) arg;

	pthread_mutex_lock(&car_lock);

	cout << "Southbound car # " << car -> carNo << " arrives at the tunnel." << endl;

	if (!(curNCarsInTunnel < maxNCarsInTunnel && curNSBCarsInTunnel < maxNSBCarsInTunnel)) {
		totwaitNCars++;
		while (!(curNCarsInTunnel < maxNCarsInTunnel && curNSBCarsInTunnel < maxNSBCarsInTunnel)) {
			pthread_cond_wait(&wake_up, &car_lock);
		}
	}

	curNCarsInTunnel++;
	curNSBCarsInTunnel++;

	cout << "Southbound car # " << car -> carNo << " enters the tunnel." << endl;

	pthread_mutex_unlock(&car_lock);

	sleep(car -> travelTime);

	pthread_mutex_lock(&traffc_lock);

	totalNSBCars++;
	curNCarsInTunnel--;
	curNSBCarsInTunnel--;
	cout << "Southbound car # " << car -> carNo << " exits the tunnel." << endl;
	pthread_cond_signal(&wake_up);
	pthread_cond_broadcast(&wake_up);
	pthread_mutex_unlock(&traffc_lock);

	pthread_exit((void*) 0);
} // carSB

int main() {

	ifstream infile;
	string line, filename;
	cout << "Enter the file name: ";
	cin >>  filename;
	infile.open(filename);

	if (!infile) {
		cerr << "Unable to open the file " + filename + "\n";
		exit(0);
	}

	int cntline = 0;
	while (getline(infile, line)) {
		cntline++;
	}

	infile.close();
	infile.open(filename);
	string input[cntline];
	cntline = 0;
	while (getline(infile, line)) {
		input[cntline] = line;
		cntline++;
	}

	infile.close();

	maxNCarsInTunnel = stoi(input[0]);
	maxNNBCarsInTunnel = stoi(input[1]);
	maxNSBCarsInTunnel = stoi(input[2]);

	cout << "Maximum number of cars in the tunnel: " << maxNCarsInTunnel << endl;
	cout << "Maximum number of northbound cars: " << maxNNBCarsInTunnel << endl;
	cout << "Maximum number of southbound cars: " << maxNSBCarsInTunnel << endl;

	string direction = "";

	pthread_t cartid[maxNCarsInTunnel];

	int error = pthread_mutex_init(&car_lock, NULL);

	if (error) cout << "Mutex init failed." << endl;

	int SBCarNo = 1, NBCarNo = 1;
	for (int i = 3; i < cntline; i++) { // skip 1st 3 lines

		struct Car *car = new struct Car;
		int timeDelay = 0;

		direction = entry(input[i], 1, ' ');
		timeDelay = stoi(entry(input[i], 0, ' '));
		sleep(timeDelay);

		car -> travelTime = stoi(entry(input[i], 2, ' '));

		if (direction == "N") {
			car -> carNo = NBCarNo++;
			error = pthread_create(&cartid[i], NULL, carNB, (void *) car);
			if (error) cout << "Thread can not be created!" << endl;
		} else if (direction == "S") {
			car -> carNo = SBCarNo++;
			error = pthread_create(&cartid[i], NULL, carSB, (void *) car);
			if (error) cout << "Thread can not be created!" << endl;
		}
	}

	for (int i = 3; i < cntline; i++) {
		pthread_join(cartid[i], NULL);
	}

	pthread_mutex_destroy(&car_lock);

	cout << totalNNBCars << " northbound car(s) crossed the tunnel." << endl;
	cout << totalNSBCars << " southbound car(s) crossed the tunnel." << endl;
	cout << totwaitNCars << " car(s) had to wait." << endl;
	exit(0);
	return 0;
}