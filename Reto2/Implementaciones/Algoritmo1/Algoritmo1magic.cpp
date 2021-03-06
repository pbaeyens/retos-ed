#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

const char OPERADORES[] = {'+', '-', '*', '/'};   // Se va a indicar la suma con 0, la resta con 1, el producto con 2 y el cociente con 3

// Calcula la diferencia entre dos enteros
inline int Diferencia(int a, int b) {
   return b<=a ? a-b : b-a;
}

// Calcula el cociente de dos enteros, según se describió el cociente en la teoría
inline int Cociente(int a, int b) {
	return a%b == 0 ? a/b : (b%a == 0 ? b/a : 0);
}

// Hace a op b
inline int Opera(int a, int b, short int op) {
   if (op == 0)   return a+b;
   if (op == 1)   return Diferencia(a, b);
   if (op == 2)   return a*b;
   if (op == 3)   return Cociente(a, b);
}

// Estructura para el elemento L_k
struct Nodo {
   int previo1, previo2;       // Posiciones de los nodos anteriores
   short int op;               // Operación con la que se ha llegado a este nodo
   short int generacion;       // Puede obtenerse de usados, pero está aquí para acelerar
   unsigned short int usados;  // El i-ésimo bit menos significativo indica si el i-ésimo número disponible está usado
   int valor;
};

// Clase para la lista L
class VectorNodos {
private:
   static const int CAPACIDAD = 177699;	// Máximo tamaño posible para 5 generaciones sin considerar la poda por asociatividad más 993
   Nodo* nodos;
   int elementos;
   int comienzo_generacion[7];
   bool barrido[1000];		// Para las posiciones 100~999, almacena si algún elemento ha tomado ese valor

public:
   VectorNodos()
   :elementos(0), comienzo_generacion{}, barrido{} {
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
   int ComienzoGeneracion(short int g) const {
      return comienzo_generacion[g];
   }
   // Almacena la próxima posición en la que se guardarán nodos como la posición de la generación indicada
   void NuevaGeneracion(short int g) {
      comienzo_generacion[g] = elementos;
   }
   // Marca un número entre 100 y 999 como obtenido
   void Marca(int n) {
   	if (n >= 100 && n < 1000)
   		barrido[n] = true;
   }
   // Devuelve true si todos los elementos entre 100 y 999 se pueden obtener (es decir, la combinación es mágica)
   bool Magica() {
   	for (int i = 999; i > 99; i--)
   		if (!barrido[i])
   			return false;
   			
   	return true;
   }
};

// Comprueba si un par de nodos proceden de un mismo número inicial (y por tanto no pueden combinarse)
inline bool SeSolapan(unsigned short int a, unsigned short int b) {
   return a&b;
}

// Indica si una operación es asociativa
inline bool Asociativa(short int k) {
   return k%2 == 0;
}

/* Evita las siguientes expresiones (también con * y /):
   (a+b)+c, porque equivale a: a+(b+c),
   (a-b)-c, porque equivale a: a-(b+c),
   a-(b-c), porque equivale a: (a+c)-b, y
   a+(b+c) si b se obtuvo antes que a, porque equivale a: b+(a+c) */
inline bool MalAsociacion(const VectorNodos &nodos, int i, int j, short int k) {
   return nodos[i].op == k || (nodos[j].op == k && (!Asociativa(k) || nodos[j].previo1 < i));
}

// Indica si el resultado de la operación coincide con el valor de uno de los nodos de los que procede
inline bool Repite(const VectorNodos &nodos, int resultado, int i, int j) {
   return resultado == nodos[i].valor || resultado == nodos[j].valor;
}

// Añade a un vector de nodos todos los que pueden obtenerse a partir de ellos
bool OtraGeneracion(VectorNodos &nodos, short int generacion) {
   nodos.NuevaGeneracion(generacion);
   int tope_i = nodos.ComienzoGeneracion((generacion+1)/2+1);
   for (int i = 0; i < tope_i; i++) {
      unsigned short int usados_i = nodos[i].usados;
      int inicio_j = max(i+1, nodos.ComienzoGeneracion(generacion-nodos[i].generacion));
      int tope_j = nodos.ComienzoGeneracion(generacion-nodos[i].generacion+1);
      for (int j = inicio_j; j < tope_j; j++)
         if (!SeSolapan(usados_i, nodos[j].usados))
            for (short int k = 0; k < 4; k++) {
               int resultado = Opera(nodos[i].valor, nodos[j].valor, k);
               if (resultado > 0 && !Repite(nodos, resultado, i, j) && !MalAsociacion(nodos, i, j, k)) {
                  nodos.Marca(resultado);
               	if (generacion != 6)
                     nodos.push_back({i, j, k, generacion, usados_i | nodos[j].usados, resultado});
               }
            }
	}

   return generacion < 6;
}

// Recrea la secuencia de operaciones con las que se ha llegado a un determinado nodo
void Recrea(const VectorNodos &vec, const Nodo &nodo) {
   if (&nodo != 0 && nodo.generacion > 1) {
      Recrea(vec, vec[nodo.previo1]);
      Recrea(vec, vec[nodo.previo2]);
      short int operador = nodo.op;
      int valor1 = vec[nodo.previo1].valor;
      int valor2 = vec[nodo.previo2].valor;
      if ((operador%2) && valor1 < valor2)
         cout << valor2 << ' ' << OPERADORES[operador] << ' ' << valor1 << " = " << nodo.valor << '\n';
      else
         cout << valor1 << ' ' << OPERADORES[operador] << ' ' << valor2 << " = " << nodo.valor << '\n';
   }
}

// Muestra la combinación actual
void ImprimeCombinacion(int disponibles[]) {	
   cout << "{";
   for(int i = 0; i < 6; i++)
      cout << (i?",":"") << disponibles[i];
   cout << "}" << endl;
}

// Comprueba si una determinada combinación es mágica
bool Cifras(int disponibles[]) {
   VectorNodos nodos;
   for (int i = 0; i < 6; i++)
      nodos.push_back({0, 0, -1, 1, (1 << i), disponibles[i]}); // El -1 en la operación es para que no se compruebe asociatividad correcta

   for (short int g = 2; OtraGeneracion(nodos, g); g++);

   if (nodos.Magica())
   	ImprimeCombinacion(disponibles);
}

// Programa principal. Comprueba todas las combinaciones en busca de las que sean mágicas
int main() {
   clock_t tini = clock();

   int posibles[14] = {1,2,3,4,5,6,7,8,9,10,25,50,75,100};
   int a, b, c, d, e, f;
   for (a = 0; a < 14; a++)
  	 for (b = a; b < 14; b++)
     for (c = b; c < 14; c++)
      for (d = c; d < 14; d++)
       for (e = d; e < 14; e++)
        for (f = e; f < 14; f++) {
           int disponibles[6] = {posibles[a],posibles[b],posibles[c],posibles[d],posibles[e],posibles[f]};
           Cifras(disponibles);
        }

   clock_t tfin = clock();

   // Muestra el tiempo transcurrido
   cout << "\nTiempo: " << (tfin-tini)/(double)CLOCKS_PER_SEC << " segundos" << endl;
}

