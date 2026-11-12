# Julve_AIO

Aplicación Todo-en-Uno de acceso rápido para **Flipper Zero** diseñada para organizar y ejecutar señales y mandos previamente guardados desde una única interfaz, sin necesidad de ir cambiando entre las aplicaciones nativas de Sub-GHz, IR, RFID, NFC, iButton o Picopass.

> La aplicación no incluye funciones de captura o grabación: utiliza archivos previamente guardados en la tarjeta SD mediante las aplicaciones oficiales del Flipper Zero.

---

## Características principales

- **Soporte multiprotocolo** en un mismo menú:
  - **Sub-GHz (`.sub`)**:
    - Las señales estáticas y `RAW` se transmiten directamente con una pulsación.
    - Las claves con **rolling code / protocolo dinámico** abren una vista dedicada con toda la información decodificada (protocolo, bits, key, serial, contador `Cnt`, botón, fabricante, frecuencia y modulación) y botón **Send**, actualizando automáticamente el contador tanto en pantalla como en el archivo tras cada envío.
  - **Infrarrojos (`.ir`)**:
    - Importa mandos `.ir` completos en un único archivo (sin dividirlos por botones).
    - Al abrir un archivo `.ir`, renderiza el mando completo con la interfaz de botones vertical del Flipper Zero original; mantener pulsado `OK` repite la señal de forma continua mientras siga presionado.
    - Compatible con módulo IR externo por OTG.
  - **RFID 125 kHz (`.rfid`)**, **NFC (`.nfc`)**, **iButton (`.ibtn`)** y **Picopass / iClass (`.picopass`)**.
- **Playlists (`.qpl`)**: encadena múltiples señales secuencialmente con pausas configurables y duraciones personalizadas.
- **Enlaces simbólicos (`.ql`)**: referencia archivos en cualquier ubicación de la tarjeta SD sin duplicarlos (ideal para mantener sincronizados los contadores de claves rolling code o compartir mandos IR).
- **Gestión de archivos desde la propia app**: renombrar, borrar, importar archivos completos, importar enlaces y crear grupos/carpetas.
- **Interfaz personalizable**: modo horizontal o vertical, iconos, cabeceras, duraciones de emisión y visibilidad de archivos ocultos.

---

## Navegación y controles

### Lista principal y grupos
- `Up` / `Down`: desplazarse por los elementos.
- `OK`:
  - Sobre una **carpeta**: entra en el grupo.
  - Sobre un **mando IR (`.ir`)**: abre la pantalla completa del mando con todos sus botones.
  - Sobre una **clave Sub-GHz rolling code (`.sub`)**: abre la pantalla con toda la información de la clave y el botón `Send`.
  - Sobre cualquier otra **señal** (`.sub` estático/RAW, `.rfid`, `.nfc`, `.ibtn`, `.picopass`) o **playlist (`.qpl`)**: la transmite/ejecuta directamente (el LED parpadea en azul durante la emisión y en rojo si hay error).
- `Back`: vuelve a la carpeta anterior o sale de la aplicación en el nivel raíz.
- Pulsación larga en `Right`: abre el menú contextual de la carpeta/elemento actual:
  - **Rename**: renombra el archivo manteniendo su extensión (y `.ql` si es un enlace).
  - **Delete**: elimina archivos o carpetas vacías previa confirmación.
  - **Import Here**: abre el explorador de la SD y copia el archivo seleccionado (incluidos mandos `.ir` completos) a la carpeta actual.
  - **Import Link Here**: crea un archivo de enlace `.ql` apuntando al archivo original en la SD.
  - **Create Group**: crea una nueva subcarpeta en la ubicación actual.

### Vista de mando IR (`.ir`)
- `Up` / `Down`: seleccionar botón del mando.
- `OK` (pulsar o mantener): transmite el comando IR seleccionado mientras se mantenga pulsado.
- `Back`: regresa a la lista.

### Vista Sub-GHz Rolling Code (`.sub`)
- `OK` (pulsar o mantener): transmite la clave (`Send` / `Sending`), guarda el nuevo contador en el archivo y refresca la información en pantalla al soltar.
- `Up` / `Down`: desplaza las líneas de información si el protocolo muestra más de 5 líneas.
- `Back`: regresa a la lista.

---

## Organización de archivos y nombres

La estructura de menús refleja directamente los archivos y carpetas dentro de:

```text
/ext/apps_data/julve_aio/
```

Reglas de visualización y ordenación:
- **Orden personalizado**: anteponer `XX_` (dos dígitos seguidos de guion bajo, p. ej. `00_Garaje.sub`, `01_Tele.ir`) fuerza el orden alfabético deseado; el prefijo `XX_` se oculta en pantalla.
- **Formato legible**: los guiones bajos `_` se muestran como espacios y no se muestran las extensiones.
- **Archivos ocultos**: los archivos o carpetas que empiezan por `.` no se muestran salvo que `Show Hidden` esté activado (pero siguen pudiendo usarse desde playlists). El archivo de configuración `.julve_aio.conf` se omite siempre.

---

## Playlists (`.qpl`)

Un archivo `.qpl` es un fichero de texto plano con una acción por línea:
- Líneas que comienzan por `#` se tratan como comentarios y las líneas vacías se ignoran.
- `pause <ms>`: detiene la ejecución durante los milisegundos indicados.
- Ruta a un archivo de señal (absoluta o relativa a la carpeta de la playlist).
- Para `.sub`, `.rfid`, `.nfc`, `.ibtn` y `.picopass` se puede indicar opcionalmente una duración en ms tras un espacio (sobrescribe el valor por defecto de los ajustes solo para esa línea).
- En playlists, los archivos `.ir` reproducen el primer comando del archivo.

Ejemplo:

```text
# Rutina de llegada
.luz_exterior.sub
pause 1500
/ext/subghz/Garaje.sub 1000
Portal.rfid 3000
```

---

## Ajustes de la aplicación

Accesibles desde la opción **Settings** al final del menú raíz (se guardan en `/ext/apps_data/julve_aio/.julve_aio.conf`):

- **Layout**: `Vert` (vertical) o `Horiz` (horizontal).
- **Show Icons**: muestra u oculta los iconos por tipo de señal.
- **Show Headers**: muestra u oculta la cabecera superior para ganar espacio en pantalla.
- **SubGhz / RFID / NFC / iButton / Picopass Duration**: duración de transmisión/emulación directa (`500 ms` a `10 sec`).
- **IR Ext Module**: habilita alimentación OTG para módulos IR externos cuando hay uno conectado.
- **Show Hidden**: muestra u oculta archivos y carpetas cuyo nombre comienza por `.`.
- **About**: muestra versión e información de la app.

---

## Compilación

Con [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) instalado:

```bash
ufbt
```

Al compilar, el paquete `.fap` se generará en `dist/julve_aio.fap`.

---

## Créditos

Este proyecto es un fork de [Quac!](https://github.com/rdefeo/quac).

