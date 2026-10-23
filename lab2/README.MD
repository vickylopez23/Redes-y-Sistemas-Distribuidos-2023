# Laboratorio 2: Aplicación Servidor


## Nombres de los integrantes 
- Tomas Ponce 
- Franco Molina 
- Victoria Lopez

## Estructuración del servidor y las decisiones de diseño tomadas

Estructuramos el servidor de este modo: Primero la conexion es manejada por el objeto Connection, que es ejecutada por un Thread para manejar multiples conexiones. Dentro de este objeto manejamos el parseo y una vez parseado el comando se abstrae de la ejecucion del mismo a un objeto de CommandExecutor. La idea es simple, el comando es ejecutado por este executor que se encarga de determinar la ejecucion a realizar en base al comando (representado por una Enum de Python), luego en todo caso devuelve un CommandResult que representa el resultado del comando, talque al transformarlo a String representa la respuesta en el formato deseado
Finalmente la conexion se encarga de devolver este CommandResult como String, y solo tiene una particularidad que si todo esta Ok y el comando es QUIT se encarga de cerrar la conexion, esto dentro de Connection y no en CommandExecutor .Lo util de esta decision de diseño es separar las distintas partes del proceso de respuesta el servidor en "modulos" que se encargan de una parte

## Dificultades Encontradas 

En terminos de dificultades encontradas  principalmente fue la forma en que habia que realizar el parseo, ya que una de las caracteristicas del servidor es que un comando puede estar disperos en varios "mensajes" donde mensaje se define como el momento en que el servidor recibe un conjunto de datos. Pero esto no es asi, tal como en el ejemplo del test de quit que se envia por partes. A causa de esto hubo que pensar distintos y buscar parsear el comando de un modo secuencial, viendo caracter por caracter

## Preguntas

### 1. ¿Qué estrategias existen para poder implementar este mismo servidor pero con capacidad de atender múltiples clientes simultáneamente?

- Crear un nuevo proceso para cada nueva conexión de cliente: Esta estrategia consiste en crear un nuevo proceso para cada nueva conexión de cliente. De esta forma, cada cliente tendría su propio proceso para manejar su conexión. Esta estrategia se puede implementar utilizando la función fork en sistemas Unix o la función multiprocessing.Process en Python.

- Crear un nuevo hilo para cada nueva conexión de cliente: Esta estrategia es similar a la anterior, pero en lugar de crear un nuevo proceso para cada cliente, se crea un nuevo hilo. Esta estrategia se puede implementar utilizando la clase Thread en Python.

- Utilizar un modelo de concurrencia basado en eventos: Esta estrategia consiste en utilizar un bucle de eventos para manejar múltiples conexiones de cliente simultáneamente. En lugar de crear un nuevo proceso o hilo para cada cliente, el servidor espera por eventos de entrada y salida en todas las conexiones y maneja cada evento de manera asíncrona. Este modelo de concurrencia se puede implementar utilizando bibliotecas como asyncio en Python.


La elecciónde cada estrateguia dependdra e los requerimientos específicos del servidor y de las características del lenguaje de programación utilizado.

### ¿Qué diferencia hay si se corre el servidor desde la IP “localhost”, “127.0.0.1” o la ip “0.0.0.0”?

127.0.0.1 es la dirección del protocolo de Internet de bucle (IP) también conocida como localhost. Esta direccion se utiliza para establecer una conexión IP a la misma máquina o computadora que usa el usuario final, cualquier servidor que se ejecute en "localhost" o "127.0.0.1" sólo estará disponible para el propio equipo.

La dirección IP "0.0.0.0" se refiere a todas las interfaces de red disponibles en el equipo Es una meta-dirección utilizada como placeholder (marcador de posición) para no especificar una dirección en particular. En el contexto del servidor, esto quiere decir que si la máquina donde corre el servidor tiene dos (o más) direcciones IP y el servidor escucha en la IP 0.0.0.0, el mismo puede ser alcanzado a través de cualquiera de esas direcciones IP.

En resumen, si se ejecuta un servidor en "localhost" o "127.0.0.1", sólo estará disponible en el equipo local, mientras que si se ejecuta en "0.0.0.0", estará disponible en toda la red local.