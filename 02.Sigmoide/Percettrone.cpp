#include <iostream>
#include <algorithm>

typedef float real;
using namespace std;

#define SIGM(x) 1.0 / (1.0 + std::exp(-(x)))
#define DERS(x) (x) * (1-(x))

class Percettrone
{
	public:
		int m_n_input;
		real* m_weights;
		real m_bias;
	public:
		Percettrone(int p_n_input)
		{
			m_n_input = p_n_input;
			m_weights = new real[m_n_input];
			for (int i=0; i<m_n_input; ++i)
				m_weights[i] = 0;
			m_bias = 0;
		}
		
		real A(real* p_inputs)
		{
			real output=0;
			for (int i=0; i<m_n_input; ++i)
				output+= p_inputs[i] * m_weights[i];
			return output + m_bias;
		}
		
		real T(real* p_inputs)
		{
			return SIGM( A(p_inputs) );
		}
		
		void Apprendimento(int p_n_examples, real* p_inputs, real* p_answers, real p_eta, int p_epochs, real p_min_error)
		{
			int current_epoch=0;
			real current_error;
			real weights_correction[m_n_input];
			real bias_correction;
			do
			{
				current_error = 0;
				fill(&weights_correction[0], &weights_correction[m_n_input], 0);
				bias_correction = 0;
				for (int i=0; i<p_n_examples; i++)
				{
					real output = T(&p_inputs[i*m_n_input]);
					current_error += std::pow(p_answers[i] - output, 2);
					for (int j=0; j<m_n_input; j++)
						weights_correction[j]+= (output - p_answers[i]) * DERS(output) * p_inputs[i*m_n_input+j];
					bias_correction += (output - p_answers[i]) * DERS(output);
				}
				for (int i=0; i<m_n_input; i++)
					m_weights[i] -= weights_correction[i] * p_eta / p_n_examples;
				m_bias -= bias_correction * p_eta / p_n_examples;
				current_error /= 2*p_n_examples;
			} while (++current_epoch<=p_epochs && current_error > p_min_error);
			cout<<"Apprendimento terminato all'epoca "<<current_epoch<<" con errore "<<current_error<<endl;
		}
};

int main(int argc, char* argv[]) {
	if (argc != 2)
	{
		cout<<"Si prega di passare tramite riga di comando un intero per indicare il problema.\n";
		cout<<"Tipo di problema:\n";
		cout<<"0: Fiera\n";
		cout<<"1: AND\n";
		cout<<"2: OR\n";
		cout<<"3: XOR\n";
		return (1);
	}
	
	Percettrone* percy;
	real* inputs;
	real* risposte;
	
	int type = atoi(argv[1]);
	cout<<"Problema: ";
	if (type == 0)
	{
		cout<<"fiera"<<std::endl;
		percy = new Percettrone(3); 
		inputs =new real[24]{
			0,0,0, //caso a e b
			0,0,1, //caso a
			0,1,0, //caso a
			0,1,1, //caso a
			1,0,0, //caso b
			1,0,1, //caso c
			1,1,0, //caso c
			1,1,1};//caso c
		risposte=new real[8]{0,0,0,0,0,1,1,1};

		for (int i=0; i<8; i++)
			cout<<"Caso: "<<i<<", Risultato ottenuto: "<<percy->T(&inputs[i*3])<<
				  ", risposta voluta: "<<risposte[i]<<endl;
	
		percy->Apprendimento(8, inputs, risposte, 0.5, 100, 0);
	
		for (int i=0; i<8; i++)
			cout<<"Caso: "<<i<<", Risultato ottenuto: "<<percy->T(&inputs[i*3])<<
				  ", risposta voluta: "<<risposte[i]<<endl;
	}
	else
	{
		percy = new Percettrone(2);
		inputs = new real[8]{0,0, 0,1, 1,0, 1,1};
		switch (type)
		{
			 /*AND*/ case 1: risposte=new real[4]{0,0,0,1}; cout<<"AND"<<std::endl; break;
			 /*OR*/  case 2: risposte=new real[4]{0,1,1,1}; cout<<"OR"<<std::endl; break;
			 /*XOR*/ case 3: risposte=new real[4]{0,1,1,0}; cout<<"XOR"<<std::endl; break;
			 default: return(1);
		}
		for (int i=0; i<4; i++)
			cout<<"Caso: "<<i<<", Risultato ottenuto: "<<percy->T(&inputs[i*2])<<
				  ", risposta voluta: "<<risposte[i]<<endl;
	
			percy->Apprendimento(4, inputs, risposte, 1, 100, 0);

		for (int i=0; i<4; i++)
			cout<<"Caso: "<<i<<", Risultato ottenuto: "<<percy->T(&inputs[i*2])<<
				  ", risposta voluta: "<<risposte[i]<<endl;
	}
	return 0;
}
