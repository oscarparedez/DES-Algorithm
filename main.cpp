/*
*----------------------------------------
* main.cpp
* ---------------------------------------
* UNIVERSIDAD DEL VALLE DE GUATEMALA
* CC3056 - Programacion de Microprocesadores
* Aut.: Guido Padilla 19200, Oscar Paredez 19109
* Ver.: 1.0
* Algoritmo DES personalizado con implementacion de hilos
*----------------------------------------
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <pthread.h>	
#include <fstream> //file processing
#include <cstdlib> //exit function
#include <bits/stdc++.h>
#include <bitset>
#include <algorithm>
#include <boost/algorithm/string.hpp>
	    
/*----- Variables globales compartidas -----*/
std::string subLlaves[16];
int counter = 0, thread_count=16, releaser=0;
std::string * Lectura;
std::string * Escritura;
int contadorLectura=0;
int contadorEscritura=0;
int contadorLecturaBinario=0;
int contadorEscrituraBinario=0;

bool bandera=true;
std::string key; 
pthread_mutex_t barrier_mutex,release_mutex,escritura_mutex;  
int size;

/*
** ---------------  FUNCION QUE CONVIERTE UNA CADENA DE TEXTO A CADENA BINARIA  --------------- **
*/

std::string stringBinario(std::string cadena) {
    std::string binaryString = "";
    for (char& _char : cadena) {
        binaryString +=std::bitset<8>(_char).to_string();
    }
    return binaryString;
}

/*
** ---------------  FUNCION QUE CONVIERTE UNA CADENA BINARIA A CADENA DE TEXTO  --------------- **
*/

std::string binarioString(std::string s){
  std::stringstream sstream(s);
  std::string output;
  while(sstream.good())
  {
      std::bitset<8> bits;
      sstream >> bits;
      char c = char(bits.to_ulong());
      output += c;
  }
  return output;
}

/*
** ---------------  FUNCION DE PERMUTACION   --------------- **
*/

std::string permut(std::string cadena, int* matriz, int cantBits){ 
	std::string per = ""; 
	for (int i = 0; i < cantBits; i++) { 
		per += cadena[matriz[i] - 1]; 
	} 
	return per; 
} 

/*
** ---------------  LECTURA DEL ARCHIVO FUENTE.TXT  --------------- **
*/

std::string lecturaArchivo(){
  std::ifstream t("Fuente.txt");
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

/*
** ---------------  HILO LECTURA PARA ENCRIPTAR  --------------- **
*/

void *HiloLectura(void * arg){
  int my_rank = (long)arg;
  contadorLectura = 0;
  std::string * arrayGrupos64;

	std::string cadenaFuente = lecturaArchivo();
  std::string cadenaFuenteBinario = stringBinario(cadenaFuente);
  
  if(cadenaFuenteBinario.size()<64){
    
    int residuo = 64-cadenaFuenteBinario.size();
    std::string cadenaRellenada = "";
    for(int i=0; i<(residuo/8); i++){
      cadenaRellenada+="00100000";
    }

    cadenaFuenteBinario += cadenaRellenada;
    arrayGrupos64 = new std::string[1];
    arrayGrupos64[0] = cadenaFuenteBinario;

  } else if(cadenaFuenteBinario.size()>=64){
    
    double longitudCadenaBinarios = cadenaFuenteBinario.size();
    double division = longitudCadenaBinarios/64.0;
    int cantidadGruposTotal = std::ceil(division);
    int cantidadGruposEnteros = (std::floor(cadenaFuenteBinario.size())/64);
    
    arrayGrupos64 = new std::string[cantidadGruposTotal];

    for(int i=0;i<cantidadGruposEnteros;i++){
      arrayGrupos64[i] = cadenaFuenteBinario.substr((i)*64,64);
    }

    std::string bitsRestantes = cadenaFuenteBinario.substr(cantidadGruposEnteros*64, cadenaFuenteBinario.size()-1);

    if((bitsRestantes).size() > 0){
      
      int residuo = 64-(bitsRestantes).size();
      std::string cadenaRellenada = "";
      for(int i=0; i<(residuo/8); i++){
        cadenaRellenada+="00100000";
      }

      bitsRestantes += cadenaRellenada;
      arrayGrupos64[cantidadGruposEnteros] = bitsRestantes;

    }


  }
  double longitudCadenaBinarios = cadenaFuenteBinario.size();
  double division = longitudCadenaBinarios/64.0;
  int cantidadGruposTotal = std::ceil(division);

  size=cantidadGruposTotal;

  Lectura = arrayGrupos64;
  Escritura = new std::string[size];

  bandera = false;

  pthread_mutex_lock(&release_mutex);
	releaser++;
	pthread_mutex_unlock(&release_mutex);

  pthread_exit(0);
}

/*
** ---------------  HILO ESCRITURA PARA ENCRIPTAR  --------------- **
*/

void *HiloEscritura(void * arg){
  int my_rank = (long)arg;
  char c[1];
  std::ofstream myFile ("salida.bin", std::ios::out | std::ios::binary);
  int i = 0;
  while(contadorEscritura!=thread_count-2);
  while(i<size){
    char temporal[64];
    strcpy(temporal, Escritura[i].c_str());
    myFile.write(temporal, 64);
    i++;
  }
  
  myFile.close();
  pthread_mutex_lock(&release_mutex);
	releaser++;
	pthread_mutex_unlock(&release_mutex);
  pthread_exit(0);
}

/*
** ---------------  OPEARCION XOR  --------------- **
*/

std::string operacion_XOR(std::string texto_1, std::string texto_2){
  std::string texto_final= "";
  for(int i = 0; i<texto_1.size();i++){
    std::stringstream ss;
    ss<<(texto_1[i]^texto_2[i]);
    texto_final += ss.str();
  }
  return texto_final;
}

/*
** ---------------  ENCRIPTAR  --------------- **
*/

std::string encriptar(std::string pt, std::string rkb[]) 
{ 
    // Hexadecimal to binary 
  
    // Permutacion Inicial que se realiza con los 64 bits de la muestra 
    int permutacionInicial[64] = { 58, 50, 42, 34, 26, 18, 10, 2, 
                             60, 52, 44, 36, 28, 20, 12, 4, 
                             62, 54, 46, 38, 30, 22, 14, 6, 
                             64, 56, 48, 40, 32, 24, 16, 8, 
                             57, 49, 41, 33, 25, 17, 9, 1, 
                             59, 51, 43, 35, 27, 19, 11, 3, 
                             61, 53, 45, 37, 29, 21, 13, 5, 
                             63, 55, 47, 39, 31, 23, 15, 7 }; 
    // Permutacion Inicial 
    pt = permut(pt, permutacionInicial, 64); 
  
    // Splitting 
    std::string left = pt.substr(0, 32); 
    std::string right = pt.substr(32, 32); 
  
    // Expansion de 32 bits derechos de los 64 bits totales (32 bits -> 48 bits) 
    int permutacionExpansion[48] = { 32, 1, 2, 3, 4, 5, 4, 5, 
                      6, 7, 8, 9, 8, 9, 10, 11, 
                      12, 13, 12, 13, 14, 15, 16, 17, 
                      16, 17, 18, 19, 20, 21, 20, 21, 
                      22, 23, 24, 25, 24, 25, 26, 27, 
                      28, 29, 28, 29, 30, 31, 32, 1 }; 
  
// Tabla de las S-Boxes (***MODIFICACION***) (En la ronda #1 se utiliza la sbox #1 y asi sucesivamente)
//Bit #1 y #2 se suman y se convierte a decimal (fila de la matriz). Bits #3, #4, #5, #6 se juntan (columna de la matriz)
    int sboxes[8][4][16] = {
      
                          {1, 10, 8, 6, 10, 4, 0, 6, 14, 5, 10, 14, 1, 9, 12, 6,
                           0, 4, 13, 11, 1, 6, 14, 4, 5, 9, 6, 10, 2, 3, 0, 10,
                           10, 11, 6, 0, 13, 14, 14, 7, 14, 6, 5, 3, 1, 10, 3, 7,
                           7, 10, 7, 2, 5, 11, 5, 5, 0, 6, 0, 6, 12, 13, 9, 2},

                          {3, 14, 12, 1, 2, 13, 9, 0, 1, 11, 7, 10, 5, 9, 8, 9, 
                           7, 11, 1, 2, 8, 4, 0, 8, 3, 1, 2, 0, 12, 9, 5, 14,
                           13, 3, 1, 10, 5, 12, 0, 3, 7, 1, 8, 7, 7, 8, 2, 4,
                           2, 4, 13, 3, 3, 8, 4, 0, 6, 1, 8, 12, 12, 9, 5, 11},

                          {7, 9, 0, 6, 4, 4, 10, 4, 10, 11, 0, 4, 12, 12, 6, 12, 
                           3, 9, 4, 10, 12, 0, 0, 2, 14, 12, 12, 7, 9, 2, 11, 13, 
                           12, 0, 3, 1, 14, 4, 2, 1, 1, 4, 9, 2, 4, 3, 1, 11, 
                           3, 4, 7, 3, 11, 14, 4, 4, 14, 12, 5, 14, 14, 13, 2, 8},
                             
                          {3, 7, 9, 10, 10, 6, 8, 10, 8, 2, 14, 14, 9, 0, 0, 3, 
                           0, 8, 8, 11, 8, 9, 11, 14, 10, 12, 6, 14, 11, 8, 11, 9, 
                           6, 13, 6, 13, 8, 13, 2, 7, 10, 3, 1, 11, 4, 14, 3, 12, 
                           10, 2, 11, 0, 11, 3, 13, 14, 7, 5, 12, 9, 7, 4, 12, 4},
                              
                          {12, 14, 11, 9, 2, 6, 0, 12, 3, 9, 0, 8, 8, 10, 12, 2, 
                           5, 12, 11, 1, 9, 0, 10, 3, 12, 11, 1, 1, 11, 7, 3, 2, 
                           6, 12, 5, 14, 8, 6, 13, 8, 3, 8, 8, 8, 12, 14, 12, 1, 
                           9, 9, 5, 0, 9, 11, 8, 13, 10, 0, 13, 6, 6, 14, 4, 5},
                           
                          {9, 13, 6, 13, 2, 0, 12, 9, 10, 10, 12, 7, 5, 10, 8, 0, 
                           8, 10, 12, 11, 9, 10, 13, 11, 5, 13, 12, 11, 9, 7, 8, 5, 
                           9, 6, 2, 3, 5, 7, 1, 5, 13, 6, 4, 4, 7, 12, 10, 6, 
                           5, 6, 1, 5, 12, 13, 7, 11, 13, 1, 14, 1, 6, 11, 5, 10},
                           
                          {8, 6, 4, 5, 8, 13, 12, 9, 9, 14, 8, 12, 8, 5, 5, 6, 
                           9, 5, 12, 2, 8, 6, 11, 14, 7, 2, 4, 4, 9, 11, 3, 3, 
                           0, 0, 2, 14, 12, 3, 1, 4, 7, 10, 4, 8, 9, 3, 13, 6, 
                           2, 6, 8, 10, 12, 10, 14, 8, 2, 8, 3, 10, 12, 4, 7, 9},
                           
                          {5, 5, 11, 12, 6, 2, 9, 5, 2, 11, 11, 1, 13, 2, 6, 12, 
                           14, 9, 8, 10, 5, 2, 6, 7, 4, 14, 7, 13, 13, 6, 10, 2, 
                           8, 5, 9, 11, 0, 5, 5, 10, 8, 10, 12, 0, 13, 2, 1, 4, 
                           8, 13, 13, 7, 14, 1, 10, 2, 11, 14, 12, 5, 13, 11, 4, 10}
                           
                         }; 
  
    // Permutacion que se realiza tras hacer el calculo con la sbox 
    int permutacionSboxes[32] = { 16, 7, 20, 21, 
                    29, 12, 28, 17, 
                    1, 15, 23, 26, 
                    5, 18, 31, 10, 
                    2, 8, 24, 14, 
                    32, 27, 3, 9, 
                    19, 13, 30, 6, 
                    22, 11, 4, 25 }; 
  
    for (int i = 0; i < 8; i++) { 
        // Expansion D-box 
        std::string right_expanded = permut(right, permutacionExpansion, 48); 
  
        // XOR RoundKey[i] and right_expanded 
        std::string x = operacion_XOR(rkb[i], right_expanded); 
  
        // S-boxes 
        std::string op = ""; 
        for (int i = 0; i < 8; i++) { 
            int row = 2 * int(x[i * 6] - '0') + int(x[i * 6 + 1] - '0'); 
            int col = 8 * int(x[i * 6 + 2] - '0') + 4 * int(x[i * 6 + 3] - '0') + 2 * int(x[i * 6 + 4] - '0') + int(x[i * 6 + 5] - '0'); 
            int val = sboxes[i][row][col]; 
            op += char(val / 8 + '0'); 
            val = val % 8; 
            op += char(val / 4 + '0'); 
            val = val % 4; 
            op += char(val / 2 + '0'); 
            val = val % 2; 
            op += char(val + '0'); 
        } 
        // Straight D-box 
        op = permut(op, permutacionSboxes, 32); 
  
        // XOR left and op 
        x = operacion_XOR(op, left); 
  
        left = x; 
  
        // Swapper 
        if (i != 7) { 
            swap(left, right); 
        } 
        
    } 
  
    // Combination 
    std::string combine = left + right; 
  
    // Permutacion Final (Inversa de la Inicial)  
    int permutacionFinal[64] = { 40, 8, 48, 16, 56, 24, 64, 32, 
                           39, 7, 47, 15, 55, 23, 63, 31, 
                           38, 6, 46, 14, 54, 22, 62, 30, 
                           37, 5, 45, 13, 53, 21, 61, 29, 
                           36, 4, 44, 12, 52, 20, 60, 28, 
                           35, 3, 43, 11, 51, 19, 59, 27, 
                           34, 2, 42, 10, 50, 18, 58, 26, 
                           33, 1, 41, 9, 49, 17, 57, 25 }; 
  
    // Final Permutation 
    std::string cipher = permut(combine, permutacionFinal, 64); 
    return cipher; 
} 

/*
** ---------------
** FUNCION DEL HILO BARRERA BUSY WAIT
** ---------------
*/
 void *barrera_busyWait(void * arg){
   std::string binarioTexto;
   long j=0;
	 int my_rank = (long)arg;

   std::string textoTemporal;
   while(bandera);
   while(my_rank<size){

   textoTemporal = Lectura[my_rank];

   textoTemporal = encriptar(textoTemporal,subLlaves);

   pthread_mutex_lock(&escritura_mutex);
   Escritura[my_rank] = textoTemporal;
   pthread_mutex_unlock(&escritura_mutex);
   my_rank+=(thread_count-2);
   }

   pthread_mutex_lock(&escritura_mutex);
	 contadorEscritura++;
   pthread_mutex_unlock(&escritura_mutex);
	 releaser++;
	 pthread_mutex_unlock(&release_mutex);
   
	 pthread_exit(0);
}

/*
** ---------------  FUNCION DE RIGHT SHIFT  --------------- **
*/

std::string rightShift(std::string texto, int number){
  std::string temporal = "";
  for(int i = 0; i<number;i++){
    temporal += texto[27];
    for(int j = 0;j<27;j++){
      temporal += texto[j];
    }
    texto = temporal;
    temporal ="";
  }
  return texto;
}

/*
** ---------------  GENERACION DE LAS LLAVES  --------------- **
*/

void generacionLlaves(){

  //Permutacion Definida #1 (Contraccion de 64 bits a 56 bits de la llave)
  int permutacion1[56] = {57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 
					              10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36, 
					              63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22, 
					              14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4}; 

  // Permutacion Definida #2 (Contraccion de los 28 bits resultantes del RCS de ambas partes de la llave[56 bits->48 bits])
  int permutacion2[48] = {14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10, 
						            23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2, 
						            41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48, 
						            44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32}; 

  // Cantidad de right circular shifts que se realizan dependiendo del # de ronda
  int shift_table[8] = {1, 1, 2, 2, 
                        2, 1, 1, 2}; 

  std::string binarioLlave;
  binarioLlave = stringBinario(key);
  binarioLlave = permut(binarioLlave, permutacion1, 56);

  std::string izquierda, derecha;
  izquierda = binarioLlave.substr(0, 28);
  derecha = binarioLlave.substr(28,28);

  for(int i=0;i<8;i++){
    izquierda = rightShift(izquierda, shift_table[i]);
    derecha = rightShift(derecha, shift_table[i]);

    subLlaves[i] = permut(izquierda+derecha, permutacion2, 48);
  }
}

/*
** ---------------  FUNCION QUE REALIZA EL DESENCRIPTADO MEDIANTE EL REVERSE  --------------- **
*/

void desencriptar(){
  std::reverse(subLlaves,subLlaves + 8);
}

/*
** ---------------  FUNCION QUE LEE EL ARCHIVO DE CADENA BINARIA  --------------- **
*/

std::string lecturaArchivoBinario(){
  std::ifstream t("salida.bin");
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

/*
** ---------------  HILO DE LECTURA PARA DESENCRIPTAR  --------------- **
*/

void *HiloLecturaBinario(void * arg){
  int my_rank = (long)arg;
  contadorLecturaBinario = 0;
  std::string * arrayGrupos64Binario;

	std::string cadenaFuenteTexto = lecturaArchivoBinario();

    int cantidadGruposTotal = std::ceil((cadenaFuenteTexto.size())/64);
    int cantidadGruposEnteros = (std::floor(cadenaFuenteTexto.size())/64);

    arrayGrupos64Binario = new std::string[cantidadGruposTotal];

    for(int i=1;i<=cantidadGruposEnteros;i++){
      arrayGrupos64Binario[i-1] = cadenaFuenteTexto.substr((i-1)*64,i*64);
    }

  size = std::ceil((cadenaFuenteTexto.size()))/64;
  Lectura = arrayGrupos64Binario;
  Escritura = new std::string[size];

  bandera = false;

  pthread_mutex_lock(&release_mutex);
	releaser++;
	pthread_mutex_unlock(&release_mutex);

  pthread_exit(0);
}

/*
** ---------------  HILO ESCRITURA PARA DESENCRIPTAR  --------------- **
*/

void *HiloEscrituraBinario(void * arg){
  int my_rank = (long)arg;
  char c[1];

  std::ofstream myFile("FuenteOriginal.txt");
  int i = 0;
  while(contadorEscritura!=thread_count-2);
  while(i<size){
    if(i==(size-1)){
      std::string ultimaCadena = binarioString(Escritura[size-1]).substr(0, binarioString(Escritura[size-1]).length()-1);
      boost::trim_right(ultimaCadena);
      myFile<<ultimaCadena;
    } else {
      myFile<<binarioString(Escritura[i]).substr(0, binarioString(Escritura[i]).length()-1);
    }
    i++;
  }
  
  myFile.close();
  pthread_mutex_lock(&release_mutex);
	releaser++;
	pthread_mutex_unlock(&release_mutex);
  pthread_exit(0);
}

/*
** ---------------  FUNCION DEL HILO BARRERA BUSY WAIT PARA DESENCRIPTAR  --------------- **
*/

 void *barrera_busyWait2(void * arg){
   std::string binarioTexto;
   long j=0;
	 int my_rank = (long)arg;
   
	 //char * bufferTemporal;
   std::string textoTemporal;
   while(bandera);
   while(my_rank<size){
	 //-- Busy-Wait barrier -->

   textoTemporal = Lectura[my_rank];

   
   textoTemporal = encriptar(textoTemporal,subLlaves);
   
   pthread_mutex_lock(&escritura_mutex);
   Escritura[my_rank] = textoTemporal;
   pthread_mutex_unlock(&escritura_mutex);
   my_rank+=(thread_count-2);
   }
	 //<-- end Busy-Wait barrier
   pthread_mutex_lock(&escritura_mutex);
	 contadorEscritura++;
   pthread_mutex_unlock(&escritura_mutex);
	 releaser++;
	 pthread_mutex_unlock(&release_mutex);
   
	 pthread_exit(0);
} 

/*
** ---------------  MAIN  --------------- **
*/

int main(int argc, char *argv[]){
	pthread_t hijo[thread_count];
  std::string opcion;

	std::cout<<"INGRESE UNA LLAVE DE 8 CARACTERES:"<<std::endl;
  std::cin>>key;

  std::cout<<"INGRESE UNA OPCION"<<std::endl<<std::endl<<"1. ENCRIPTAR TEXTO"<<std::endl<<std::endl<<"2. DESENCRIPTAR CADENA BINARIA"<<std::endl;

  std::cin>>opcion;
  //key = "asdfghjk";
  
  generacionLlaves();

	//Mutex init
	pthread_mutex_init(&barrier_mutex,NULL); 
	pthread_mutex_init(&release_mutex,NULL);
	
  if(opcion=="1"){
    for(long i = 0; i < thread_count; i++)
	  {
      if(i==thread_count-1){
        pthread_create(&hijo[i],NULL,HiloLectura,(void *)i);
      }
      if(i<thread_count-2){
        pthread_create(&hijo[i],NULL,barrera_busyWait,(void *)i);
      }
      if(i==thread_count-2){
        pthread_create(&hijo[i],NULL,HiloEscritura,(void *)i);

      }
	  }
  }
  if(opcion=="2"){
    desencriptar();
    for(long i = 0; i < thread_count; i++)
    {
      if(i==thread_count-1){
        pthread_create(&hijo[i],NULL,HiloLecturaBinario,(void *)i);
      }
      if(i<thread_count-2){
        pthread_create(&hijo[i],NULL,barrera_busyWait2,(void *)i);
      }
      if(i==thread_count-2){
        pthread_create(&hijo[i],NULL,HiloEscrituraBinario,(void *)i);
      }
	  }


  }

	while(releaser<thread_count);
	printf("LOS DATOS SE HAN PROCESADO. REVISE EL ARCHIVO!\n\n");
	exit(0);
}