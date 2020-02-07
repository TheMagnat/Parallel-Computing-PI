
#include <iostream>
#include <gmp.h>

#include <gmpxx.h>

#include <cmath>

#include <cstdio>

#include <vector>
#include <thread>
#include <fstream>

#include <chrono>


//Précision qui permet d'avoir le nombre de decimal que l'on veut
#define PRECI 3.4

//Stoque le nombre de thread.
size_t nbThread_;

//Contient tout les resultats de la suite de BBP.
std::vector<mpf_class> allResults_;

//Stoque le résultat finale
mpf_class finalResult_;







/*
On prèpare des classes pour chacunes des constantes du calcul (1, 2, 4, 5, 6, 8).
Cela parmet de ne pas regénérer de classe è chaque itération de la boucle principale.
Elles sont initializé avec la bonne précision dans initialize.
*/
mpf_class un;
mpf_class deux;
mpf_class quatre;
mpf_class cinq;
mpf_class six;
mpf_class huit;


//Initialize les constantes avec la bonne précision
void initialize(size_t k){
	size_t preci = k*PRECI;

	un		= mpf_class(1.0, preci);
	
	deux	= mpf_class(2.0, preci);

	quatre	= mpf_class(4.0, preci);

	cinq	= mpf_class(5.0, preci);

	six		= mpf_class(6.0, preci);

	huit 	= mpf_class(8.0, preci);

}


/*
Fonction principale lancé par les thread, calcul la suite de BBP en sautant de nbThread,
et remplis allResults.
*/
void piThread(size_t k, size_t index){

	//On prépare la précision
	size_t preci = k*PRECI;

	//La taille de saut
	size_t step(nbThread_);

	/*
		Initialisation de 16 puissance step,
	*/
	mpf_t powAdd_;
	mpf_init2(powAdd_, preci);
	mpf_set_d(powAdd_, 16.0);
	mpf_pow_ui(powAdd_, powAdd_, step);

	mpf_class powAdd(powAdd_);

	/*
		Initialisation de 16 puissance index,
		On multiplira à chaque tour de boucle cette valeur par powAdd,
		cette optimisation permet de ne pas recalculer la puissance.
	*/
	mpf_t tempoPow_;
	mpf_init2(tempoPow_, preci);
	mpf_set_d(tempoPow_, 16.0);
	mpf_pow_ui(tempoPow_, tempoPow_, index);


	//On rentre tempoPow dans une classe
	mpf_class tempo(tempoPow_, preci);

	/*
		Boucle principale,
		commence à l'index du thread, et avance de step (qui vaut le nombre de thread).
		De cette manière chaque thread aura un travail equivalent en coût.
		À chaque tour on multiplira tempo par powAdd pour obtenir 16 puissance l'index courant (16^i)
	*/
	for(size_t i(index); i < k; i += step, tempo *= powAdd){

		//On stoque le résultat de 8 * 1 pour ne pas le recalculer à chaque fois
		mpf_class huitFoisI(huit * mpf_class(i, preci));

		//Les quatre partie du calcul
		mpf_class first		= quatre/(huitFoisI + un);
		mpf_class second	= deux/(huitFoisI + quatre);
		mpf_class third 	= un/(huitFoisI + cinq);
		mpf_class fourth 	= un/(huitFoisI + six);

		//Le calcul entier, on stoque le resultat dans allResults.
		allResults_[i]		= (1.0/tempo) * (first - second - third - fourth);

	}

}


void start(size_t nbThread, size_t nbDecimal){

	//On prépare la taille de allResults au nombre de décimale
	allResults_.resize(nbDecimal, mpf_class(0.0, nbDecimal*4));
	nbThread_ = nbThread;


	std::vector<std::thread> allThread;
	allThread.reserve(nbThread);
	
	//On lance les thread
	for(size_t i(0); i < nbThread; ++i){
		allThread.emplace_back(piThread, nbDecimal, i);
	}

	//On join les thread
	for(size_t i(0); i < nbThread; ++i){
		allThread[i].join();
	}


	//On additionne tout les résultats :
	finalResult_.set_prec(nbDecimal*PRECI);
	finalResult_ = 0;
	for(size_t i(0); i < nbDecimal; ++i){

		allResults_[i].set_prec(nbDecimal*4);

		finalResult_ += allResults_[i];

	}


}

//Fonction pour écrire le fichier contenant PI
void writeInFile(size_t nbDecimal, std::string const& fileName){

	//On récupère le début des décimal dans cette variable (elle vaudra tout le temps 1 pour pi)
	mp_exp_t decimalStart;
	std::string stringResult(finalResult_.get_str(decimalStart));
	
	//Avec ce resize on coupe les décimals en trop
	stringResult.resize(nbDecimal);
	//On insère le point
	stringResult.insert(decimalStart, "."); //On rajoute le point

	//On ouvre le fichier
	std::ofstream file(fileName);

	//On écrit dedans
	file.write(stringResult.data(), stringResult.size());

	//On ferme le fichier
    file.close();
}


//Main, prend 2 argument au lancement du programme
int main(int argc, char const *argv[]){

	if (argc < 3) {
        printf("usage : ./exec.out [Nb Decimal] [nb Thread]\n");
        return 1;
    }

    size_t nbDecimal = std::atoi(argv[1]);
    size_t nbThread = std::atoi(argv[2]);

    //On initialize les constantes
    initialize(nbDecimal);



    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();


    //On démarre le programme
	start(nbThread, nbDecimal);


	std::cout << std::endl << "Temps de calculs pour trouver les décimals de pi jusqu'a " << nbDecimal << " décimals : " << std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - begin).count()/1000.0f << " seconds" << std::endl << std::endl;

	//On affiche les resultats dans un fichier
	writeInFile(nbDecimal, "Resultat_17806231_Pi_" + std::to_string(nbDecimal) + "_Decimal" + ".txt");

	return 0;

}