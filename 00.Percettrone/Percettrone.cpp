#include <iostream>
typedef float real;
using namespace std;

class Percettrone
{
	public:
		int m_n_input;
		real* m_weights;
		real m_threshold;
		Percettrone(int p_n_input, real* p_weights, real p_threshold)
		{
			m_n_input = p_n_input;
			m_weights = new real[m_n_input];
			for (int i=0; i<m_n_input; ++i)
				m_weights[i] = p_weights[i];
			m_threshold = p_threshold;
		}
		real A(real* p_inputs)
		{
			real output=0;
			for (int i=0; i<m_n_input; ++i)
				output= output + p_inputs[i] * m_weights[i];
			return output;
		}
		bool T(real* p_inputs) { return A(p_inputs) > m_threshold; }
		~Percettrone() { delete m_weights; }
};

int main(int argc, char* argv[])
{
	Percettrone percy(3, new real[3]{5.5,3,2.5}, 6);
	real* inputs=new real[24]{
		0,0,0, //caso a e b
		0,0,1, //caso a
		0,1,0, //caso a
		0,1,1, //caso a
		1,0,0, //caso b
		1,0,1, //caso c
		1,1,0, //caso c
		1,1,1};//caso c

	bool* risposte=new bool[8]{0,0,0,0,0,1,1,1};
	for (int i=0; i<8; i++)
		cout<<"Caso: "<<i<<", Risultato ottenuto: "<<percy.T(&inputs[i*3])<<
			  ", risposta voluta: "<<risposte[i]<<endl;
	return 0;
}
