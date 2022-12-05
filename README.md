# MPI Curso


El objetivo de este material es presentarte los fundamentos de MPI utilizando ejemplos en C, C++, Fortran y Python. Aunque cabe hacer la aclaración de que la librería para C y C++ es la misma, es decir no hay cambios en el uso de las funciones de MPI, sin embargo, se incluyen los ejemplos para dejar a la elección del estudiante qué lenguaje utiizar en las tareas.

Se muestra cómo compilar y ejecutar el código en cada uno de los lenguajes mencionados.

Utizaremos estándar 3.1 para la descripción de las funciones por ser la última versión liberada. Se incluyen funciones para la comunicación punto a punto, comunicación colectiva, se presentan los conceptos de grupos y comunicadores, topologías de procesos, entrada y salida paralela, entre otras.

MPI (Message-Passing Interface) es una especificación de interfaz de biblioteca de paso de mensajes. Todas las partes de esta definición son significativas. MPI aborda principalmente el modelo de programación paralela de paso de mensajes, en el que los datos se mueven desde el espacio de direcciones de un proceso al de otro proceso a través de operaciones cooperativas en cada proceso. 

MPI es una especificación, no una implementación; hay múltiples implementaciones de MPI, por ejemplo
* [Open-MPI](https://www.open-mpi.org/)
* [MPICH](https://www.mpich.org/)
* [mpi4py](https://pypi.org/project/mpi4py/)


Esta especificación es para una interfaz de biblioteca; MPI no es un lenguaje, y todas las operaciones de MPI se expresan como funciones, subrutinas o métodos, de acuerdo con los enlaces de lenguaje apropiados que, para C y Fortran, son parte del estándar MPI. 

El estándar ha sido definido a través de un proceso abierto por una comunidad de proveedores de computación paralelos, científicos informáticos y desarrolladores de aplicaciones. 

El objetivo de la interfaz de paso de mensajes, en pocas palabras, es desarrollar un estándar ampliamente utilizado para escribir programas de paso de mensajes. Como tal, la interfaz debería establecer un estándar práctico, portátil, eficiente y flexible para el paso de mensajes. 

## Material recomendado

Sitios recomendados, donde se explica el uso de funciones de MPI, de manera individual y, en algunos casos, con ejemplos de uso.

* [Open-mpi](https://www.open-mpi.org/doc/v3.1/)
* [rockiehpc.com](https://www.rookiehpc.com/mpi/docs/index.php)
* [www.bu.edu](http://www.bu.edu/tech/support/research/training-consulting/online-tutorials/mpi/)
* [MPI-Forum](https://www.mpi-forum.org/docs/)

## Convenciones

En la mayoría de los casos, las funciones MPI para C tiene la forma **MPI_Class_action_subset**, esto se originó en MPI v1. Esto cambió a partir de MPI v2 con la intención de estandarizar los nombre de las funciones de acuerdo a las siguientes reglas:

* En C, todas las funciones asociadas con una función deberán tener la forma MPI_Class_action_subset o, si el suconjunto no existe, de la forma MPI_Class_action. En Fortran, todas las rutinas tienen la forma MPI_CLASS_ACTION_SUBSET p, si el subconjunto no existe, tendrá la forma MPI_CLASS_ACTION.
* Las funciones o rutinas que no estén asociadas a una clase, deberán tener la forma MPI_Action_subset o MPI_ACTION_SUBSET, para C o Frotran.
* Los nombres de ciertas acciones se ha estandarizado. En particular, **Create** crea un objeto, **Get** recupera información sobre un objeto, **Set** establece esta informacion, **Delete** borra información, **Is** pregunta si un objeto tiene o no cierta propiedad.

Las funciones MPI están especificadas en una notación independiente del  lenguaje. Los argumentos de las funciones se marcan como IN, OUT o INOUT. El significado de esa notación es 

* IN En la función se puede utilizar el valor de entrada pero no se puede actualizar el valor del argumento una vez que regrese de la llamda.
* OUT En la función se puede actualizar el valor de entrada, pero no utilizar el valor de entrada.
* INOUT La función puede usar el valor y actualizarlo.

Todas las funciones MPI se definen primero en esta notación independiente del lenguaje y después se presentan las implementaciones en los lenguajes mencionados.
