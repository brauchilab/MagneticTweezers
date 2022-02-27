# MagneticTweezers

Instrumentos de manipulación de una sola molécula que utilizan un campo magnético para aplicar fuerza a una perla magnética atada a una biomolécula. 

El sistema consiste de cabezal "Magnetic Head Finder" (Brush Industries, 902836) el cual es controlado una fuente de corriente bipolar que puede suministrar hasta +/-1A. Este controlador está basado en Teensy 3.2. Via USB, a través de comando serial o a través GUI escrita en Qt, el usuario puede ejecutar distintos perfiles y protocolos de corriente.

# 1. Hardware

## 1.1. Controlador de pinzas magnéticas
![Alt text](/IMG/MTD.png)
## 1.2. Esquemático del controlador
![Alt text](/IMG/SCHM.png)
## 1.3. Cabezal magnético 
![Alt text](/IMG/HEAD.png)

(Brush Industries, 902836)

# 2. Software
## 2.1. GUI
![Alt text](/IMG/GUI.png)

## 2.2. Comandos serial

Los comandos toletan mayúsculas/minúsculas. Los parámetros pueden ser separados por comas o espacios. Cada comando debe terminar CR (0x0D) y LF (0x0A),
NOTE: Los canales están numerados con 1 y 2.
**AYUDA**
```
  ? 
```
- Imprime la lista de comandos

**Current**
```
  I c i 
```
- Ajusta la corriente en el canal 'c' a 'i' amperes (-1.0 a +1.0) 
- 

**Onda Sinusoidal**
```
  S f a o 
```
- Configura una señal sinusoidal, donde:
    - f = frecuencia en Hz (0.1 to 100.0)
    - a = peak de corriente en amp (0.0 to +1.0)
    - o = offset en amp (-1.0 to +1.0)
    NOTE: El peak de corriente + offset no debe superar los +- 1.0A
``` 
  S T c 
```
- Ejecuta el comando previamete asignado en el canal 'c' cuando la señal en el canal TTL de trigger es ALTA (+5V). El comando se detiene cuando es BAJA (0V).
```
  S c 
```
-  Ejecuta la última configuración ingresada para la señal sinusoidal en el canal 'c'. S 0 detendrá cualquier configuración previa. 
``` 
  S 
```
-  Muestra la ayuda con este comando. 

**Lista**
```
    L i t 
 ```
-  Ingresa un valor en una lista de corrientes, en donde: Enter a value into a current list where:
    - i = Corriente en amps (-1.0 to +1.0)
    - t = Tiempo e ejecución en milisegundos. time to run the output at this current in milliseconds
    NOTE: Si se asigna como tiempo '0', toda la lista de corriente quedará limpia. 
```
 L T c 
```
- Ejecuta la lista en el canal 'c' cuando la señal en el canal trigger TTL es ALTA. Parará cuando ésta sea BAJA. 
 ```
  L c
 ```
- Ejecuta la lista en el canal 'c'. 
- ‘L 0’ detendrá cualquier lista que se esté ejecutando.
```
  L 
```
- Muestra ayuda con este comando.

**Rampa**
```
    R i t 
```
- Ingresa un valor en la lista de rampa, en donde:
    - i = corriente en amperes (-1.0 to +1.0)
    - t = Tiempo (ms) que demora ir desde un punto previo al nuevo punto en la rampa.
    NOTE: Un tiempo de 0 limpiará la lista-
```   
    R T c
```
-  Ejecuta la lista de puntos de la rampa en el canal 'c' cuando la señal en la entrada TTL de Trigger sea ALTA (+V). Ésta se detendrá cuando la señal sea BAJA (0V).
```
    R c 
```
- Ejecuta la lista en el canal 'c'. 'R 0' detendrá cualquier protocolo activo.
```
    R
```
- Muestra ayuda con este comando.
