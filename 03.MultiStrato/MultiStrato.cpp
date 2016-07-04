#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>

#define SIGM(x) 1.0 / (1.0 + std::exp(-(x)))
#define DERS(x) (x) * (1-(x))

typedef float real;

class NeuralNetwork
{
	private:
		int m_n_layers;
		int* m_n_neurons;
		real** m_weights;
		real** m_outputs;
	
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
				int nWeights = m_n_neurons[i-1] * p_n_neurons[i];
				m_weights[i] = new real[nWeights];
				for (int j=0; j<nWeights; j++)
					m_weights[i][j] = rand() % 50 / 25.0 - 1.0;
				m_outputs[i] = new real[p_n_neurons[i]+1];
				m_outputs[i][p_n_neurons[i]] = 1;
			}
		}
		
		void Output(real* p_inputs, real* p_outputs)
		{
			for (int i=0; i<m_n_neurons[0]-1; i++)
				m_outputs[0][i] = p_inputs[i]; 
			for (int i=1; i<m_n_layers; i++)
			{
				int nNeurons = m_n_neurons[i]-1;
				int nNeuronsPrev = m_n_neurons[i-1];
				std::fill(&m_outputs[i][0], &m_outputs[i][nNeurons], 0); //L'ultimo è il bias          
				for (int j=0; j<nNeuronsPrev; j++)
				{
					real out = m_outputs[i-1][j];
					for (int k=0; k<nNeurons; k++)
						m_outputs[i][k] += m_weights[i][k*nNeuronsPrev+j] * out; 
				}
				for (int j=0; j<nNeurons; j++)
					m_outputs[i][j] = SIGM(m_outputs[i][j]);
			}
			for (int i=0; i<m_n_neurons[m_n_layers-1]-1; i++)
				p_outputs[i] = m_outputs[m_n_layers-1][i];
		}
		
		void Apprendimento(int p_n_examples, real** p_inputs, real** p_answers, real p_eta, int p_epochs, real p_min_error)
		{
			real** weightsCorrection = new real*[m_n_layers];
			real** derivative = new real*[m_n_layers];
			real* aOutput = new real[m_n_neurons[m_n_layers-1]-1];
			real current_error=1;
			int current_epoch=0;
			int nNeurons, nNeuronsPrev;
			for (int i=1; i<m_n_layers; i++)
			{
				weightsCorrection[i] = new real[m_n_neurons[i-1] * (m_n_neurons[i]-1)];
				derivative[i] = new real[m_n_neurons[i]-1];
			}
			do
			{
				current_error = 0;
				for (int i=1; i<m_n_layers; i++)
						std::fill(&weightsCorrection[i][0], &weightsCorrection[i][m_n_neurons[i-1]*(m_n_neurons[i]-1)], 0.0);
				for (int ex=0; ex<p_n_examples; ex++)
				{
					for (int i=1; i<m_n_layers; i++)
						std::fill(&derivative[i][0], &derivative[i][m_n_neurons[i]], 0.0);
					nNeurons = m_n_neurons[m_n_layers-1]-1;
					nNeuronsPrev = m_n_neurons[m_n_layers-2];
					Output(p_inputs[ex], aOutput);
					
					 for (int i=0; i<nNeurons; i++)
					{
						current_error += std::pow(p_answers[ex][i] - aOutput[i], 2);
						derivative[m_n_layers-1][i] = (aOutput[i] - p_answers[ex][i]) * DERS(aOutput[i]);
					}
					for (int i=m_n_layers-1; i>1; i--)
					{
						nNeurons = m_n_neurons[i]-1;
						nNeuronsPrev = m_n_neurons[i-1];

						for (int j=0; j<nNeurons; j++)
						{
							real der = derivative[i][j];
							for (int k=0; k<nNeuronsPrev; k++)
								weightsCorrection[i][j*nNeuronsPrev+k] += der * m_outputs[i-1][k];  
						}
						
						for (int j=0; j<nNeuronsPrev-1; j++)
						{
							for (int k=0; k<nNeurons; k++)
								derivative[i-1][j] += (m_weights[i][k*(nNeuronsPrev)+j] * derivative[i][k]);
							derivative[i-1][j]*=DERS(m_outputs[i-1][j]);
						}
					}
					nNeurons=m_n_neurons[1]-1;
					nNeuronsPrev = m_n_neurons[0];
					for (int j=0; j<nNeurons; j++)
					{
						real der = derivative[1][j];
						for (int k=0; k<nNeuronsPrev; k++)
							weightsCorrection[1][j*nNeuronsPrev+k] += der * m_outputs[0][k];  
					}
				}
				for (int i=1; i<m_n_layers; i++)
				{
					int nWeights = m_n_neurons[i-1] * (m_n_neurons[i]-1);
					for (int j=0; j<nWeights; j++)
						m_weights[i][j] -= weightsCorrection[i][j] * p_eta / p_n_examples;
				}
				current_error/=2*p_n_examples;
				if (current_epoch % 100 == 0 && current_epoch <501) 
					std::cout<<"("<<current_epoch<<"), Errore:\t\t"<<current_error<<std::endl;
				if (current_epoch == 501)
					std::cout<<"..."<<std::endl;
				if (current_epoch % 100 == 0 && current_epoch >20000 && current_epoch <20501) 
						std::cout<<"("<<current_epoch<<"), Errore:\t\t"<<current_error<<std::endl;
				if (current_epoch == 20501)
					std::cout<<"..."<<std::endl;
				if (current_epoch % 100 == 0 && current_epoch >49500) 
						std::cout<<"("<<current_epoch<<"), Errore:\t\t"<<current_error<<std::endl;
			} while (++current_epoch<=p_epochs && current_error > p_min_error);

			delete[] weightsCorrection;
			delete[] derivative;
		}
		void dumpInfo()
		{
			std::cerr<<"Numero strati: "<<m_n_layers<<std::endl;
			for (int i=0; i<m_n_layers; i++)
			{
				std::cerr<<"Strato: "<<i<<", n° neuroni: "<<m_n_neurons[i]<<std::endl;
				std::cerr<<"Pesi strato "<<i<<std::endl;
				for (int j=0; j<(m_n_neurons[i] - 1) * m_n_neurons[i-1]; j++)
				{
					std::cerr<<m_weights[i][j]<<std::endl;
				}
			}
		}
		
		~NeuralNetwork()
		{
			delete[] m_outputs[0];
			for (int i=1; i<m_n_layers; i++)
			{
				delete[] m_weights[i];
				delete[] m_outputs[i];
			}
			delete[] m_n_neurons;
			delete[] m_weights;
			delete[] m_outputs;
		}
};

int main()
{
	//stderr = fopen("/dev/null", "w");
	int* neurons = new int[2]{2, 1};
	NeuralNetwork net(2, neurons);
	
	real** training_set = new real*[4] {new real[2]{0,0}, new real[2]{0,1}, new real[2]{1,0}, new real[2]{1,1} };
	real** outputs = new real*[4] {new real[1]{0}, new real[1]{1}, new real[1]{1}, new real[1]{0}};
	
	for (int i=0; i<4; i++)
	{
		float res;
		net.Output(training_set[i], &res);
		std::cout<<res<<" ";
	}
	std::cout<<std::endl;
	
	net.Apprendimento(4, training_set, outputs, 3.5, 50000, 0);
	
	std::cout<<"Outputs: "<<std::endl;
		
	for (int i=0; i<4; i++)
	{
		float res;
		net.Output(training_set[i], &res);
		std::cout<<res<<" ";
	}
	std::cout<<std::endl;

	//delete[] training_set;
	//delete[] output;
	delete[] neurons;
}
	
	
