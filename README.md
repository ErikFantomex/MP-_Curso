# MPI Curso


El objetivo de este material es presentarte los fundamentos de MPI utilizando ejemplos en C, C++, Fortran y Python. Aunque cabe hacer la aclaración de que la librería para C y C++ es la misma, es decir no hay cambios en el uso de las funciones de MPI, sin embargo, se incluyen los ejemplos para dejar a la elección del estudiante qué lenguaje utiizar en las tareas.

Se muestra cómo compilar y ejecutar el código en cada uno de los lenguajes mencionados.

Utizaremos estándar 3.1 para la descripción de las funciones por ser la última versión liberada. Se incluyen funciones para la comunicación punto a punto, comunicación colectiva, se presentan los conceptos de grupos y comunicadores, topologías de procesos, entrada y salida paralela, entre otras.

MPI (Message-Passing Interface) es una especificación de interfaz de biblioteca de paso de mensajes. Todas las partes de esta definición son significativas. MPI aborda principalmente el modelo de programación paralela de paso de mensajes, en el que los datos se mueven desde el espacio de direcciones de un proceso al de otro proceso a través de operaciones cooperativas en cada proceso.

MPI es una especificación, no una implementación;
