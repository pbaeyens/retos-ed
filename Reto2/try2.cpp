#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

char operadores[] = {'+', '-', '*', '/'};   // Se va a indicar la suma con 0, la resta con 1, el producto con 2 y el cociente con 3

inline int Diferencia(int a, int b) {
   return b<=a ? a-b : b-a;
}

inline int Maximo(int a, int b) {
	return b<=a ? a : b;
}

// Hace a op b. Aprovecha que siempre hay como mucho una resta y una division permitida
inline int Opera(int a, int b, short int op) {
   if (op == 0)   return a+b;
   if (op == 1)   return Diferencia(a, b);
   if (op == 2)   return a*b;
   if (op == 3)   return (a == 0 || b == 0) ? 0 : (a%b == 0 ? a/b : (b%a == 0 ? b/a : 0));
}

struct Nodo {
   int previo1, previo2; // Posiciones de los nodos anteriores
   short int op;         // Operación con la que se ha llegado a este nodo
   int generacion;       // Puede obtenerse de usados, pero está aquí para acelerar
   unsigned int usados;  // El i-ésimo bit menos significativo indica si el i-ésimo número disponible está usado
   int valor;
};

class VectorNodos {
private:
	static const int CAPACIDAD = 1144386;	// Máximo tamaño posible para 6 generaciones sin considerar la poda por asociatividad
	Nodo* nodos;
	int elementos;
	int comienzo_generacion[7];
	
public:
	VectorNodos()
	:elementos(0), comienzo_generacion({0,0,0,0,0,0,0}) {
		nodos = new Nodo[CAPACIDAD];
	}
	~VectorNodos() {
		delete[] nodos;
	}
	const Nodo& operator[](int pos) const {
		return nodos[pos];
	}
	void push_back(const Nodo& nodo) {
		nodos[elementos++] = nodo;
	}
	int size() const {
		return elementos;
	}
	// Indica en qué posición se inician los elementos de una generación
	int ComienzoGeneracion(int g) {
		return comienzo_generacion[g];
	}
	// Almacena la próxima posición en la que se guardarán nodos como la posición de la generación indicada
	int NuevaGeneracion(int g) {
		comienzo_generacion[g] = elementos;
	}
};

// Comprueba si un par de nodos proceden de un mismo número inicial
inline bool SeSolapan(unsigned int a, unsigned int b) {
   return a&b;
}

inline bool Asociativa(short int k) {
	return k%2 == 0;
}

/*	Evita las siguientes expresiones (también con * y /):
	(a+b)+c, porque equivale a: a+(b+c), 
	(a-b)-c, porque equivale a: a-(b+c),
	a-(b-c), porque equivale a: (a+c)-b, y
	a+(b+c) si b se obtuvo antes que a, porque equivale a: b+(a+c) */
inline bool MalAsociacion(VectorNodos &nodos, int i, int j, short int k) {
	return nodos[i].op == k || (nodos[j].op == k && (!Asociativa(k) || nodos[j].previo1 < i));
}

// Añade a un vector de nodos todos los que pueden obtenerse a partir de ellos
bool OtraGeneracion(VectorNodos &nodos, int &mas_cercano, int objetivo, int generacion) {
   nodos.NuevaGeneracion(generacion);
   Nodo nuevo;
   int tope_i = nodos.ComienzoGeneracion((generacion+1)/2+1);
   for (int i = 0; i < tope_i; i++) {
      unsigned int usados_i = nodos[i].usados;
      int inicio_j = Maximo(i+1, nodos.ComienzoGeneracion(generacion-nodos[i].generacion));
      int tope_j = nodos.ComienzoGeneracion(generacion-nodos[i].generacion+1);
      for (int j = inicio_j; j < tope_j; j++)
         if (!SeSolapan(usados_i, nodos[j].usados))
            for (short int k = 0; k < 4; k++) {
               int resultado = Opera(nodos[i].valor, nodos[j].valor, k);
               if (resultado != 0 && resultado != nodos[i].valor && resultado != nodos[j].valor && !MalAsociacion(nodos, i, j, k)) {
                  nuevo = {i, j, k, generacion, usados_i | nodos[j].usados, resultado};
                  nodos.push_back(nuevo);
                  int diferencia = Diferencia(nodos[nodos.size()-1].valor, objetivo);
                  if (diferencia < Diferencia(nodos[mas_cercano].valor, objetivo)) {
                     mas_cercano = nodos.size()-1;
                     if (diferencia == 0) return false;
                  }
               }
            }
	}

   return generacion < 6;
}

// Recrea la secuencia de operaciones con las que se ha llegado a un determinado nodo
void Recrea(VectorNodos &vec, const Nodo &nodo) {
   if (&nodo != 0 && nodo.generacion > 1) {
      Recrea(vec, vec[nodo.previo1]);
      Recrea(vec, vec[nodo.previo2]);
      short int operador = nodo.op;
      int valor1 = vec[nodo.previo1].valor;
      int valor2 = vec[nodo.previo2].valor;
      if ((operador%2) && valor1 < valor2)
         cout << valor2 << ' ' << operadores[operador] << ' ' << valor1 << " = " << nodo.valor << '\n';
      else
         cout << valor1 << ' ' << operadores[operador] << ' ' << valor2 << " = " << nodo.valor << '\n';
   }
}

// Resuelve un problema y muestra la solución
void Cifras(int solucion, int disponibles[]) {
   VectorNodos nodos;
   Nodo nodo;
   for (int i = 0; i < 6; i++)
      nodos.push_back(nodo = {0, 0, -1, 1, (1 << i), disponibles[i]}); // El -1 en la operación es para que no se compruebe asociatividad correcta

   int mas_cercano = 0;
   for (int g = 2; OtraGeneracion(nodos, mas_cercano, solucion, g); g++);

   Recrea(nodos, nodos[mas_cercano]);
}

// Programa principal. Genera los números al azar si no se pasan siete parámetros
int main(int argc, char* argv[]) {
	int disponibles[6];
	int solucion;
	if (argc == 8) {
		solucion = atoi(argv[7]);
	 	for (int i = 0; i < 6; i++)
	      disponibles[i] = atoi(argv[i+1]);
	} else {
	   int posibles[14] = {1,2,3,4,5,6,7,8,9,10,25,50,75,100};
	
	   srand(time(0));
	   solucion = (rand()%900) + 100;
	   
	 	for (int i = 0; i < 6; i++)
	      disponibles[i] = posibles[rand()%14];
	}
	
   cout << "[";
   for(int i = 0; i < 6; i++)
      cout << (i?",":"") << disponibles[i];
   cout << "] --> " << solucion << '\n' << endl;
   Cifras(solucion, disponibles);
}