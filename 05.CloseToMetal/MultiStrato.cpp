#include <iostream>
#include <chrono>
#include "clWrapper.hpp"

#define __SIGMOID__(x) 1.0 / ( 1.0 + std::exp(-x) )
#define __D_SIGMOID__(x) x * ( 1 - x )

#define IMAGES 1000
#define MINI 10

typedef float real;
real** images;
real** answers;
FILE* res;
FILE* err;
FILE* idxErr;

class NeuralNetwork
{
	int m_n_layers;
	int* m_n_neurons;
	real** m_weights;
	real** m_outputs;
	real** m_weights_correction;
	real** m_part_derivative;
	
	void HiddenOutput(real* p_inputs)
	{
		clwSetInput(p_inputs, m_n_neurons[0]);
		clwClearOutput(m_n_layers, m_n_neurons);
		for (int i=1; i<m_n_layers; i++)
		{
			int nNeurons = m_n_neurons[i];
			int nNeuronsPrev = m_n_neurons[i-1];
			clwCalcOutput(i, nNeuronsPrev, nNeurons);
			clwSigmOutput(i, nNeurons);
		}
	}

	public:
		NeuralNetwork(int p_n_layers, int* p_n_neurons)
		{
			m_n_layers=p_n_layers;
			m_n_neurons = new int[p_n_layers];
			m_weights = new real*[p_n_layers];
			m_outputs = new real*[p_n_layers];
			m_n_neurons[0] = p_n_neurons[0]+1;
			m_outputs[0] = new real[p_n_neurons[0]+1];
			m_outputs[0][p_n_neurons[0]] = 1;
			for (int i=1; i<p_n_layers; i++)
			{
				m_n_neurons[i] = p_n_neurons[i]+1;
				int nWeights = m_n_neurons[i-1] * (m_n_neurons[i]-1);
				m_weights[i] = new real[nWeights];
				for (int j=0; j<nWeights; j++)
					m_weights[i][j] = rand() % 50 / 25.0 - 1;
				m_outputs[i] = new real[p_n_neurons[i]+1];
				m_outputs[i][p_n_neurons[i]] = 1;
			}
			m_weights_correction = new real*[m_n_layers];
			m_part_derivative = new real*[m_n_layers];
			for (int i=1; i<m_n_layers; i++)
			{
				m_weights_correction[i] = new real[m_n_neurons[i-1] * (m_n_neurons[i]-1)];
				m_part_derivative[i] = new real[m_n_neurons[i]-1];
			}
			clwSetup(m_n_layers, m_n_neurons, m_outputs, m_weights, m_weights_correction, m_part_derivative);
		}
		
		void Output(real* p_inputs, real* p_outputs)
		{
			HiddenOutput(p_inputs);
			clwReadOutput(m_n_neurons[m_n_layers-1], p_outputs);
		}
		
		void Apprendimento(int p_n_examples, real** p_inputs, real** p_answers, float p_eta, int p_epochs, real p_min_error)
		{
			float err=p_min_error+1;
			int currEpoch=0;
			int resto = p_n_examples % MINI;
			while  (err > p_min_error && currEpoch < p_epochs)
			{
				err=0;
				clwClearWCorr(m_n_layers, m_n_neurons);
				for (int i=0; i<p_n_examples; i++)
				{
					HiddenOutput(p_inputs[i]);
					clwClearPartDer(m_n_layers, m_n_neurons);
					err+= clwCalcCost(m_n_neurons[m_n_layers-1], p_answers[i]);
					for (int j=m_n_layers-1; j>1; j--)
					{
						clwCalcWCorr(j, m_n_neurons[j-1], m_n_neurons[j]);
						clwCalcPartDer(j-1, m_n_neurons[j-1], m_n_neurons[j]);
					}
					clwCalcWCorr(1, m_n_neurons[0], m_n_neurons[1]);
					if (i%MINI == (MINI-1))
					{
						for (int j=1; j<m_n_layers; j++)
						{
							clwApplyCorr(j, m_n_neurons[j-1], m_n_neurons[j], p_eta / MINI);
						}
						clwClearWCorr(m_n_layers, m_n_neurons);
					}
				}
				if (resto != 0)
					for (int j=1; j<m_n_layers; j++)
						clwApplyCorr(j, m_n_neurons[j-1], m_n_neurons[j], p_eta / resto);

				currEpoch++;
				if (currEpoch % 5 == 0)
					Test(currEpoch, true, false);
				err/=p_n_examples*2;
				std::cout<<"Epoca: "<<currEpoch<<" con errore "<<err<<std::endl;
			}
			std::cout<<"Apprendimento terminato all'epoca "<<currEpoch<<" con errore "<<err<<std::endl;
		}
		
		void Test(int epoca, bool p_toFile = false, bool p_printWrong=false)
		{
			real max;
			int best, total=0;
			real output[10];
			bool found=false;
			int correct=0;
			int numWrongs=0;
			for (int i=0; i<IMAGES; i++)
			{
				max=best=0;
				Output(images[i], output); 
				for (int j=0; j<10; j++)
					if (output[j] > max)
					{
						max = output[j];
						best = j;
					}
				found = false;
				for (int j=0; j<10; j++)
					if (answers[i][j] == 1)
					{
						correct = j;
						if (best==j)
						{
							total++;
							found = true;
						}
					}
				if (!found  && p_printWrong)
				{
					std::cout<<"Cifra n° "<<i<<" non riconosciuta correttamente: era un "<<correct<<", io ho detto "<<best<<std::endl;
					fprintf(idxErr, "%d %d %d\n", i, correct, best);
					++numWrongs;
				}
			}
			if (!p_toFile)
				std::cout<<"("<<epoca<<") Risultato rete: "<<total<<" / "<<IMAGES<<" ("<<total*100.0/IMAGES<<"%)"<<std::endl;
			else
				fprintf(res, "%d\t%d\n", epoca, total);
			if (p_printWrong)
			{
				fclose(idxErr);
				idxErr = fopen("numIdxErr", "w");
				fprintf(idxErr, "%d", numWrongs);
				fclose(idxErr);
			}
		}

		~NeuralNetwork()
		{
			clwTerminate();
			delete[] m_outputs[0];
			for (int i=1; i<m_n_layers; i++)
			{
				delete[] m_weights[i];
				delete[] m_outputs[i];
				delete[] m_weights_correction[i];
				delete[] m_part_derivative[i];
			}
			delete[] m_n_neurons;
			delete[] m_weights;
			delete[] m_outputs;
			delete[] m_weights_correction;
			delete[] m_part_derivative;
		}
};

int main()
{
	stderr = fopen("/dev/null", "w"); //Sopprime eventuali errori OpenCL (nel mio caso, dovuti a bug del driver)
	res = fopen("results", "w");
	err = fopen("error", "w");
	idxErr = fopen("idxNumb", "w");
	uint32_t dump[4];
	int* neurons = new int[3]{784, 30, 10};
	NeuralNetwork net(3, neurons);
	
	images = new real*[IMAGES];
	answers = new real*[IMAGES];

	FILE* in = fopen("train-labels.idx1-ubyte", "rb");
	fread(dump, sizeof(uint32_t), 2, in);
	for (int i=0; i<IMAGES; i++)
	{
		answers[i] = new real[10]{0,0,0,0,0,0,0,0,0,0};
		answers[i][fgetc(in)] = 1;
	}

	fclose(in);
	in = fopen("train-images.idx3-ubyte", "rb");
	fread(dump, sizeof(uint32_t), 4, in);
	for (int i=0; i<IMAGES; i++)
	{
		images[i] = new real[784];
		for (int j=0; j<784; j++)
			images[i][j] = fgetc(in) / 255.0;
	}
	fclose(in);

	net.Test(0);

	std::chrono::steady_clock::time_point clock_begin = std::chrono::steady_clock::now();
	net.Apprendimento(IMAGES, images, answers, 3, 20, 0.001);
	std::chrono::steady_clock::time_point clock_end = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration time_span = clock_end - clock_begin;
	double nseconds = double(time_span.count()) * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;

	std::cout<<"Finito\n";
	getchar();
	net.Test(0, false, true);
	
	std::cout << "Tempo impiegato: " << nseconds << " secondi."<<std::endl;
	
	scanf("%*d");
	fflush(err);
	fflush(res);
	for (int i=0; i<IMAGES; i++)
	{
		delete[] images[i];
		delete[] answers[i];
	}
	delete[] images;
	delete[] answers;
	delete[] neurons;
}
