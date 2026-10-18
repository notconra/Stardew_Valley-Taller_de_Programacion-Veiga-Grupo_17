# Especificación de Mapa (Formato TOML)

## 1. Estructura Base y Terreno

Todo archivo de mapa debe contener zona, ancho, alto y un arreglo plano `terreno` que represente la grilla completa (ancho * alto).

**Diccionario de IDs de Terreno:**
* **0:** Pasto
* **1:** Tierra
* **2:** Tierra tratada
* **3:** Límite / Pared exterior
* **4:** Árbol
* **5:** Agua
* **6:** Valla
* **7:** Pared casa
* **8:** Puerta casa
* **9:** Salida al pueblo

(Por el momento no hay más ids de elementos estáticos)

## 2. Entidades Iniciales

Las entidades se definen en un arreglo de tablas llamado `entidades`. Cada entidad necesita un `tipo` (String en mayúsculas) y coordenadas `x` e `y` basadas en la grilla del mapa.

**Tipos:**
* `JUGADOR` (Punto de aparición inicial del usuario)
* `VACA`
* `GALLINA`
* `CULTIVO_PAPA`
* `CULTIVO_FRUTILLA`
* `CULTIVO_CALABAZA`
* `CULTIVO_CEBOLLA_VERDEO`

(Por ahora, los cultivos se muestran en su estado de crecimiento máximo).
(Por el momento no hay más tipos)

## 3. Ejemplo de Archivo TOML válido

```toml
zona = "GRANJA"
ancho = 5
alto = 5


terreno = [
    3, 3, 3, 3, 3,
    3, 0, 0, 0, 3,
    3, 0, 2, 0, 3,
    3, 1, 1, 1, 3,
    3, 3, 8, 3, 3
]

[[entidades]]
tipo = "JUGADOR"
x = 2
y = 1

[[entidades]]
tipo = "VACA"
x = 1
y = 1

[[entidades]]
tipo = "CULTIVO_PAPA"
x = 2
y = 2
```