/*
 * >This app was made for the neural net training.
 * >The net is gonna be trained to get 1 as output
 *  to represent a class (1 here) and this value will
 * be given to the master probe.  
 * 
 *
 *     !--------------------MINI TUTORIAL---------------------------!
 * <1> Turn on the probes and take notes of the read values (5 for each)
 * <2> Insert these values separed by commas on the console
 * <3> Pick up 0 or 1 to repsent the class
 * <4> Define the number of epocas (1000 or above)
 * 
 */

/* 
 * File:   main.cpp
 * Author: Aurelio Buarque (abuarquemf@gmail.com)
 *
 * Created on July 5, 2015, 10:13 PM
 */

//Used libraries
#include <iostream>
#include <math.h>
#include <ctime>
#include <cstdlib>

//Used namespace
using namespace std;

const float k = 0.0001F; //math const to decrease the input values 
const float velocity = 0.05F; //learning velocity
float v11 = 0,v12 = 0,v21 = 0; //potencial activation
float x1 = 0,x2 = 0; //inputs
float d21 = 0, d11 = 0,d12 = 0; //local gradients
float yd = 0; //desired output
float y = 0; //net output
float e = 0; //output error
//weights and bias
float  w111 = 0,
        w112 = 0,
        w121 = 0,
        w122 = 0,
        w211 = 0,
        w221 = 0,
        b11 = 0,
        b12 = 0,
        b21 = 0,
        Dw111 = 0,Dw112 = 0,Dw121 = 0,Dw122 = 0,Dw211 = 0,Dw221 = 0,Db11 = 0,Db12 = 0,Db21 = 0;

/**
 * This function generates random weights
 *
*/
void randomWeights() {
	srand(time(NULL));
	double supportVector[9];
	for(int i=0;i<9;i++) {
		if((i % 2) == 0)
			supportVector[i] = (rand() % 10)/(-10);
		supportVector[i] = (rand() % 10)/10; 
	}

    w112 = supportVector[1],
    w121 = supportVector[2],
    w122 = supportVector[3],
    w211 = supportVector[4],
    w221 = supportVector[5],
    b11 = supportVector[6],
    b12 = supportVector[7],
    b21 = supportVector[8];
}

/**
 * 
 *
*/
void userInterface() {
    double d1,d2,d3;
    std::cout << "Digite o valor de uma caracterisitica para o primeiro objeto: " << std::endl;
    std::cout << "-> ";
    std::cin >> d1;
    std::cout << "Digite o valor de uma outra caracteristica para o primeiro objeto: " << std::endl;
    std::cout << "->";
    std::cin >> d2;
    std::cout << "Digite a classe que essas caracteristicas devem pertencer (1 ou 0): " << std::endl;
    std::cout << "->";
    std::cin >> d3;
    x1 = d1 * k;
    x2 = d2 * k;
    yd = d3;

}

void creator() {
    std::cout << "Artificial Neural Net Applied to Selective Process Otpmization" << std::endl;
    std::cout << "               (c) Aurelio Buarque 17/06/2015\n\n";
}

void q_fim() {
    std::cout<<"OK! Making the neural net................." << std::endl;
    std::cout<<"Getting weights values...................." << std::endl;
    std::cout<<"\n";
}

float sigmoid(float m) {
    return (float) (1/(1+exp(-m)));
}

float sigmoid_linha(float m) {
    return (float) (exp(-m)/(pow((1 + exp(-m)), 2)));
}

void forward() {
    v11 = (x1 * w111) + (x2 * w121) + b11;
    v12 = (x1 * w112) + (x2 * w122) + b12;
    v21 = (sigmoid(v11) * w211) + (sigmoid(v12) * w221) + b21;
    y = sigmoid(v21);
}

void atualizacao() {
    e = yd-y;
    d21 = (0-1) * e * sigmoid_linha(v21); //gradiente local da ultima camada

    Dw211 = (0-1) * velocity*d21 * sigmoid(v11);
    w211 += Dw211;

    Dw221 = (0-1) * velocity * d21 * sigmoid(v12);
    w221 += Dw221;

    Db21 = (0-1) * velocity * d21;
    b21 += Db21;

    d11 = sigmoid_linha(v11) * d21 * w221;
    Dw111 = (0-1) * velocity * d11 * x1;
    w111 += Dw111;

    Dw121 = (0-1) * velocity * x2 * d11;
    w121 += Dw121;

    Db11 = (0-1) * velocity * d11 * x1;
    b11 += Db11;

    d12 = sigmoid_linha(v12) * d21 * w221;
    Dw122 = (0-1) * velocity * x2 * d12;////////////q
    w122 += Dw122;

    Dw112 = (0-1) * velocity * x1 * d12;   ////////////////////////
    w112 += Dw112;

    Db12 = (0-1) * velocity * d12 * x2;
    b12 += Db12;
}


void learning() {
    float erro_total = 0;
    forward();
    atualizacao();
    float E = - ((e*e)/(2));
    std::cout<<"e= "<<e<<"  "<<" E= "<< E<<"\n\n\n";
    erro_total = erro_total + e;
}

int main(int argc, char** argv) {
    
    float accuracy = 0;
    float desiredAccuracy = 0.99;

	bool check = false;  
    creator();
    userInterface(); 

    do {
    	learning();
    	accuracy = y/yd;
    } while(accuracy < desiredAccuracy);

    check = true;
    
    q_fim();

    if(check == true) {
        //retorne os pesos
        std::cout<<"Valores de W1   \n";
        std::cout<<"W111= " << w111 << "\n";
        std::cout<<"W112= " << w112 << "\n";
        std::cout<<"W121= " << w121 << "\n";
        std::cout<<"W122= " << w122 << "\n";
        std::cout<<"W211= " << w211 << "\n";
        std::cout<<"W221= " << w221 << "\n";
        std::cout<<"---------------------\n";
        std::cout<<"Valores de bias:     \n";
        std::cout<<"b11= " << b11 << "\n";
        std::cout<<"b12= " << b12 << "\n";
        std::cout<<"b21= " << b21 << "\n";

        std::cout << "Accuracy " << accuracy << std::endl;
        std::cout << "final error: " << e << std::endl;
    }

    return 0;
}