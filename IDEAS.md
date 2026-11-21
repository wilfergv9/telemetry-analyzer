# IDEAS.md — Game Monitor (nombre de trabajo)

> Banco de ideas, decisiones y pendientes del proyecto. Se actualiza cada vez que aparece una idea nueva (anótala con fecha). Última actualización: 2026-11-19.

**Etiquetas usadas**

- **[Decidido]**: ya acordado.
- **[Plan]**: intención clara, falta validarla.
- **[Propuesta]**: sugerida, sin decidir.
- **[Futuro]**: para después del MVP.
- **[Fuera de alcance]**: se descarta por ahora.

## Índice

1. Visión
2. Contexto y objetivos
3. Funcionalidades (banco de ideas)
4. Impacto del propio monitor
5. Arquitectura
6. Tecnología: decisiones y pendientes
7. Alcance: MVP, después y fuera de alcance
8. Hoja de ruta y primeros pasos
9. Cómo validar que es útil
10. Riesgos y limitaciones
11. Aprendizaje
12. Herramientas existentes de referencia
13. Notas técnicas y correcciones
14. Pendientes y preguntas abiertas

---

## 1. Visión

**[Decidido]** Aplicación de **código abierto** que recopila, registra, analiza y visualiza el comportamiento de un videojuego y del hardware durante una sesión de juego.

**Idea original**

- Al abrir un juego, poder **elegir si la aplicación se activa** para esa sesión.
- Al terminar el juego, generar un **informe visual e interactivo**, con gráficos (FPS, temperaturas, uso de componentes) y también los **datos reales completos**, para seguir el rendimiento de cada juego.

**Frase central:** cada sesión de juego se convierte en un registro histórico que se puede consultar, explorar, analizar y comparar después.

**Cómo pensarlo**

- No es "un monitor". Es un **sistema de telemetría y análisis de videojuegos**: recopilar → sincronizar → almacenar → analizar → visualizar → comparar.
- **[Decidido]** No es otro MangoHud ni otro MSI Afterburner. MangoHud es una **fuente de datos**, no el competidor.
- **[Decidido]** Linux primero; Windows como etapa futura, con otros backends detrás de las mismas abstracciones.

**Por qué Linux primero:** expone muchas interfaces estándar (`/proc`, `/sys`, `hwmon`, DRM, journald, herramientas de los fabricantes), con mucho más acceso a logs, errores y datos del sistema.

**Matiz importante:** Linux no entrega automáticamente *todos* los datos de un juego. Lo que permite es agregar muchas fuentes de telemetría que ya existen. Capturar FPS y frametime sigue exigiendo engancharse a la API gráfica (por ejemplo, una capa de Vulkan), y eso ya lo hace MangoHud.

**Novedad, dicha con honestidad:** moderada. La captura no es nueva. La propuesta propia es la capa de **registro histórico, análisis, correlación de eventos y comparación de sesiones** sobre datos que ya existen.

---

## 2. Contexto y objetivos

Hay dos proyectos dentro de uno, y ambos importan igual:

1. **El producto:** una herramienta de telemetría y análisis de videojuegos, abierta, visual y extensible.
2. **El aprendizaje:** recuperar y construir conocimientos de Linux, sistemas operativos, arquitectura, C++, bases de datos, estadística y visualización a través de un problema real.

**[Plan]** Se piensa como proyecto universitario, pero con intención real de ser una herramienta útil.

**[Plan]** La propuesta al profesor es pronto. Se quiere **empezar a trabajar en las vacaciones**, para llegar al siguiente semestre con algo hecho y aprendido.

**[Decidido]** Regla de trabajo: **no aceptar una solución técnica que no se sepa explicar**, venga de documentación, GitHub o una IA. Se entiende antes de incorporarla.

**[Decidido]** No introducir complejidad sin una necesidad demostrada.

---

## 3. Funcionalidades (banco de ideas)

### 3.1 Activación y detección del juego

- **[Plan]** Preguntar al abrir un juego si se quiere monitorear esa sesión.
- **[Propuesta]** Un wrapper como opción de lanzamiento en Steam (`gamemon %command%`) o un detector de procesos.
- **[Plan]** Seguir el **árbol de procesos completo**, porque el juego puede correr bajo Proton/Wine, Steam, Lutris, Heroic o de forma nativa (por ejemplo, saber cuándo termina un `wine64-preloader`).
- **[Propuesta]** Integraciones por plataforma: `game/steam`, `game/lutris`, `game/generic`.

### 3.2 Recolección de datos de hardware

Registrar también **cuándo una métrica no está soportada** (`N/D`), sin inventarla.

**CPU**

- Uso total y por núcleo/hilo, frecuencia por núcleo, temperatura.
- Consumo energético, cuando esté disponible.
- Carga atribuible al proceso del juego, context switches, CPU time, affinity.

**GPU**

- Utilización, temperatura, frecuencias, VRAM usada y disponible.
- Consumo y potencia, voltaje y ventiladores cuando el hardware los exponga.
- Estado de rendimiento y **razones de throttling**.
- Utilización por motor, si el hardware lo expone.

**Memoria**

- RAM usada y disponible, memoria del proceso del juego, swap, page faults.

**Otros**

- Disco/SSD (actividad de I/O), red, procesos y otros sensores que se puedan obtener.

### 3.3 FPS y frametime

- FPS, frametime, **1% low y 0.1% low**, picos de frametime, variación y stuttering.
- FPS y frametime **no son lo mismo**: se pueden tener 100 FPS y sentir tirones.
- **[Propuesta]** Al inicio, usar los **logs de MangoHud** como fuente de frametime en el MVP.
- **[Plan, avanzado]** Más adelante, evaluar una captura propia. **[Fuera de alcance]** por ahora: capa Vulkan propia.

### 3.4 Errores, eventos y crasheos

- **[Plan]** Mostrar **qué pasó justo en el momento del crasheo** (RAM, CPU, GPU, SSD) para facilitar el diagnóstico o el análisis del usuario.
- Fuentes: código de salida o señal del proceso, `coredumpctl`, `journalctl`, `dmesg` (errores de GPU), logs de Proton (`PROTON_LOG=1`), OOM killer.
- **[Propuesta]** Para tener los últimos segundos antes de un crash: grabar de forma continua con un buffer circular y guardado incremental.
- **[Decidido]** Los datos dan **pistas**, no siempre la causa. Se presentan así.
- Ejemplo de línea de eventos: spike de frame → caída de uso de GPU → subida de un hilo de CPU → recuperación.

### 3.5 Registro histórico y sesiones

- **[Decidido]** Cada ejecución produce una **sesión independiente**, organizada por juego:

```text
Game Monitor/
├── <Juego A>/
│   ├── Session_001
│   └── Session_002
└── <Juego B>/
    └── Session_001
```

- Cada sesión conserva: identificación del juego, fecha y duración, configuración del sistema y hardware, versiones de drivers, kernel y software relevante (Proton, etc.), mediciones, eventos y errores, estadísticas calculadas.
- **[Plan]** **Informe de la configuración del juego**, "de ser posible". La versión de Proton se puede leer de Steam, pero ajustes como DLSS casi nunca se pueden leer del juego.
- **[Propuesta]** **Etiquetas y notas manuales** por sesión (por ejemplo: DLSS on/off, ray tracing, preset gráfico) para lo que no se pueda detectar.
- **[Decidido]** **Persistencia incremental** y resistente a cierres inesperados, en lugar de guardar todo al final.
- Una sesión grabada se puede abrir **meses después** y volver a analizar.
- **[Propuesta]** SQLite (con transacciones agrupadas y WAL) o archivos por sesión (`metadata.json`, `cpu.csv`, `gpu.csv`, `memory.csv`, `fps.csv`, `processes.csv`, `events.log`). Aún sin decidir.

### 3.6 Análisis y diagnóstico

Se quiere **interpretar** los datos, no solo guardarlos:

| Análisis | Qué detecta |
| --- | --- |
| Cuello de botella CPU | Caída de FPS con un núcleo al límite y GPU con margen |
| Cuello de botella GPU | GPU muy alta y CPU con margen |
| Thermal throttling | Temperatura sube, frecuencia baja, FPS baja |
| Stuttering | Percentiles bajos frente al promedio; picos de frametime |
| Presión de memoria | RAM o VRAM al límite |
| Anomalías | Comportamientos inusuales (detección de cambios en la serie) |

- **[Decidido]** Los diagnósticos se presentan como **hipótesis con nivel de confianza**, no como verdades. Una correlación no demuestra una causa.
- **[Propuesta]** Mostrar siempre acceso a los datos crudos junto al diagnóstico.
- **[Plan]** **Conclusión automática** del estilo "el sistema estuvo predominantemente limitado por GPU; no se detectaron problemas térmicos; 7 stutters asociados a picos de frametime".
- **[Propuesta]** Eventos con ventana de contexto: al detectar una caída, mostrar el estado de todas las métricas en ese instante.

### 3.7 Comparación de sesiones

Probablemente el mayor diferenciador.

- **[Plan]** Preguntas que debe responder: "cambié de Proton 10 a Proton 11, ¿mejoró o empeoró?", "activé DLSS, ¿mejoró o empeoró?". También: antes y después de un driver, otros kernels, otros ajustes gráficos, otras versiones del juego.
- Mostrar diferencias en FPS promedio, 1% low, temperaturas, consumo y estabilidad.
- **[Decidido]** Validez: una sola sesión por configuración no basta. Lo ideal es una **escena repetible** (benchmark integrado o ruta fija) y **varias corridas** por configuración.
- **[Propuesta]** Distinguir "diferencia dentro del ruido" de "diferencia significativa" en lugar de decir "mejoró" a secas.
- **[Propuesta]** **Alerta de regresión**: avisar cuando una sesión empeora frente al historial del mismo juego (por ejemplo, "tras actualizar el driver, el 1% low empeoró un X%").
- Aclaración: DLSS mejora FPS, pero no se mide la calidad de imagen.
- Ejemplo ilustrativo de contexto guardado por sesión: GPU, driver, kernel, distribución, resolución, DLSS, RT.

### 3.8 Informe e interfaz visual

**[Decidido]** Interfaz principalmente visual e interactiva; la complejidad detrás, no delante. Debe permitir **investigar**, no solo observar.

- Gráficos temporales interactivos con **zoom** sobre cualquier intervalo (por ejemplo, 00:37:12 a 00:37:15).
- **Paneles apilados con eje de tiempo compartido** (FPS arriba, luego GPU, CPU, temperaturas), con selección de métricas. Evitar 20 líneas superpuestas en una gráfica.
- Eventos marcados sobre la línea de tiempo (crasheos, caídas de FPS, throttling).
- Tarjetas de resumen: FPS promedio, 1% y 0.1% low, temperaturas máximas, uso de RAM/VRAM, cantidad de stutters.
- Histogramas, FPS vs. temperatura, throttling en la línea de tiempo.
- **Árbol de sensores agrupado por dispositivo** (inspirado en la captura de CoolerControl).
- Historial de juegos y de sesiones; comparación entre sesiones.
- **[Propuesta]** Informe como **HTML autocontenido** generado al terminar la sesión (Plotly, ECharts o uPlot), sin interfaz pesada mientras se juega.
- Tecnologías para la interfaz (Qt, GTK, Tauri y otras): **por evaluar**. Tauri y Electron son pesados durante el juego.

### 3.9 Exportación

- **[Plan]** Exportar informes en **JSON, SQLite y Markdown** para analizarlos con una IA o con otras herramientas.
- **[Propuesta]** Para IA, además del volcado completo, un **informe `.md` compacto**: estadísticas, eventos y ventanas alrededor de las anomalías. Los datos crudos son demasiado grandes para el contexto de un modelo.
- **[Propuesta]** **Quitar de la exportación** el nombre de usuario y las rutas locales.
- Otros formatos (PDF, etc.): por definir.

### 3.10 IA integrada

- **[Futuro]** Integrar una API de algún modelo (de pago o gratuito) para ayudar a diagnosticar dentro de la aplicación.
- **[Propuesta]** Que sea **opcional**, con la clave del propio usuario, avisando de que el diagnóstico puede equivocarse y cuidando la privacidad de los datos enviados.

---

## 4. Impacto del propio monitor

**[Decidido]** Requisito de primer nivel: el programa no puede convertirse en otro cuello de botella ni alterar lo que mide.

**Cómo medirlo**

- Misma escena repetible, varias corridas **sin** el monitor y varias **con** él.
- Comparar FPS promedio, 1% low y variación del frametime.
- Criterio realista: diferencia **dentro del ruido** normal entre corridas sin monitor. No se promete "0%".
- Medir también el costo propio: CPU, RAM y escrituras a disco.

**Cómo reducirlo** (todo **[Propuesta]**, a validar midiendo)

- **Muestreo escalonado:** por ejemplo, CPU/GPU cada 100–500 ms y temperaturas cada 500–1000 ms.
- **Lecturas baratas:** mantener abiertos los archivos de `/proc` y `/sys` y releerlos; usar NVML directamente y **nunca** `nvidia-smi` en bucle.
- **Sin asignaciones de memoria** en el bucle de muestreo (buffers reservados).
- **Escritura por lotes:** transacciones agrupadas y guardado cada pocos segundos, sin sincronizar a disco por muestra. Es un compromiso con la persistencia incremental.
- **Análisis solo después del juego:** durante la sesión solo se recolecta y se guarda.
- **Prioridad baja** (`nice` o planificación de baja prioridad); en CPUs híbridas (núcleos P y E), probar anclar el monitor a los núcleos E, midiendo el efecto.
- **Sin GPU propia** mientras se juega.
- **Auto-medición:** el monitor registra su propio uso de CPU y memoria en cada sesión, como evidencia verificable en el informe.
- No optimizar a ciegas: primero funcionar, luego medir dónde está el costo. C++ ayuda, pero el bajo impacto sale del diseño.

---

## 5. Arquitectura

**[Decidido]** Modular y extensible; backend separado de la interfaz (la captura funciona aunque el dashboard esté cerrado). POO cuando tenga sentido, sin clases innecesarias: importa más la modularidad.

```text
Detección del juego  →  Collectors  →  Recorder  →  Base de datos  →  Analizador  →  Informe / Dashboard
(procesos, Proton)     (CPU, GPU,      (sincroniza,    (historial de     (estadísticas,   (gráficos, historial,
                        RAM, FPS,       buffer,         sesiones y        anomalías,       comparaciones)
                        eventos)        persiste)       datos crudos)     diagnóstico)
```

**Backends por fabricante** (**[Decidido]**): una interfaz común con implementaciones NVIDIA, AMD, Intel y genérica de Linux. En C++, serían clases abstractas con métodos virtuales puros. Mismo principio después para Windows.

**Frecuencias:** FPS y frametime alta; CPU y GPU media; temperaturas y RAM más baja.

**Niveles de telemetría** (para extensibilidad; solo los dos primeros son realistas):

1. Sistema: CPU, GPU, RAM, temperaturas, procesos.
2. Renderizado: FPS, frametime, tiempos de GPU.
3. Juego: telemetría que el propio juego entregue.
4. Engine: Unity, Unreal, etc.
5. Instrumentación: datos internos mediante plugins o mods.

Los niveles 3 a 5 casi no son viables en juegos cerrados. **[Fuera de alcance]** por ahora.

**Estructura de repositorio sugerida (propuesta)**

```text
monitor/
├── core/
├── collectors/  (cpu, gpu/{nvidia,amd,intel}, memory, storage, network)
├── game/        (steam, lutris, generic)
├── analysis/
├── database/
└── ui/
```

---

## 6. Tecnología: decisiones y pendientes

| Tema | Estado |
| --- | --- |
| Plataforma inicial | **[Decidido]** Linux |
| Código fuente | **[Decidido]** Abierto |
| Windows | **[Futuro]** |
| Lenguaje principal | **[Plan]** C++, en collectors y recorder |
| Alternativas de lenguaje | Python para prototipar rápido, Rust, Java |
| Base de datos | **[Propuesta]** SQLite |
| Informe/interfaz | **[Propuesta]** HTML generado post-sesión; Qt, GTK, Tauri por evaluar |
| Fuente de FPS en el MVP | **[Propuesta]** logs de MangoHud |
| Frecuencia de muestreo | Por determinar experimentalmente |
| Formato de exportación | JSON, SQLite, Markdown planeados; resto por definir |
| Buffer en memoria | **[Propuesta]** |
| Cola lock-free, daemon propio, capa Vulkan | No adoptados |
| Licencia | Por decidir |
| Nombre del proyecto | Por decidir |

**Sobre C++ (plan del autor):** buena elección por bajo overhead, acceso directo a APIs en C (NVML, SQLite, libbpf) y portabilidad futura a Windows.

- Riesgo: aprender C++ mientras se diseña el sistema.
- Mitigación: C++ moderno y básico (`std::string`, `std::vector`, `std::ifstream`, `std::filesystem`, `std::chrono`, `std::thread`), sin plantillas avanzadas ni `new`/`delete` manual al principio, y compilar con `-Wall -Wextra` desde el inicio.
- "Mayormente C++" no significa "todo en C++": el informe puede ser HTML generado a partir de los datos.
- El **contrato entre componentes es el formato de la sesión**: si está bien definido, se puede cambiar cualquier lado.

---

## 7. Alcance: MVP, después y fuera de alcance

**MVP** — una aplicación que detecta un juego, registra una sesión de unos 10 minutos, guarda las mediciones y produce un informe visual útil. Se prueba con un solo equipo (Fedora, Intel y NVIDIA).

- Detección del proceso del juego.
- CPU, GPU, RAM y temperaturas.
- FPS y frametime desde una fuente existente.
- Almacenamiento histórico (SQLite).
- Informe visual con zoom temporal.
- Medición del impacto del propio monitor.

**Después**

- Comparación de sesiones y alertas de regresión.
- Diagnóstico automático.
- Detección avanzada de crasheos y eventos del sistema.
- Soporte completo para AMD e Intel.
- Exportación para IA.

**Futuro lejano**

- IA integrada por API.
- Windows.

**[Fuera de alcance] por ahora**

- Capa Vulkan propia.
- Telemetría interna del engine o del juego.
- Drivers o código de kernel.
- "Absolutamente todos los datos": no es realista ni conveniente (más muestreo significa más overhead). Se define por niveles y con muestreo configurable.

---

## 8. Hoja de ruta y primeros pasos

**Etapas**

1. Fundamentos de Linux: procesos, PID, PPID, `/proc`, `/sys`, permisos.
2. Primer collector: información real de un proceso y del sistema, mostrada en terminal.
3. Monitoreo de hardware: CPU, GPU, RAM, temperaturas, frecuencias, consumo.
4. Telemetría gráfica: FPS, frametime y captura de presentación de frames.
5. Registro de sesiones: almacenamiento, sincronización temporal y recuperación.
6. Análisis: estadísticas y anomalías.
7. Interfaz: dashboard e informes interactivos.
8. Expansión: comparación de sesiones, diagnóstico avanzado, más fabricantes y Windows.

**Primera tarea concreta**

- En Fedora: `ps aux`, elegir un PID y luego `cat /proc/PID/status` y `cat /proc/PID/stat`.
- Ejercicio en C++: un programa que reciba un PID, lea `/proc/PID/status` con `std::ifstream` e imprima `Name`, `State`, `Threads` y `VmRSS`.

**Prueba de viabilidad para el profesor (opcional, nivel intermedio)**

- Un programa en C++ que durante un minuto lea el uso de CPU de `/proc/stat` y una temperatura de `hwmon` y lo guarde en un CSV.
- Presentarlo como "prueba de viabilidad, no el producto". Convierte la gráfica del mockup de simulada a real.

**Plan para las vacaciones:** por hitos, donde cada etapa deje algo usable aunque no se llegue a la siguiente. Falta definir cuántas semanas hay.

---

## 9. Cómo validar que es útil

1. Hacer un primer collector pequeño y usarlo en juegos propios.
2. Con una sesión grabada, buscar algo que un overlay no habría mostrado.
3. Mostrárselo a otros jugadores de Linux y preguntarles si lo usarían.
4. Pregunta guía: **"¿por qué se trabó mi juego anoche?"**. Si el programa la responde, tiene valor real.

Si no aporta, se habrá perdido poco y aprendido bastante.

---

## 10. Riesgos y limitaciones

| Riesgo | Mitigación |
| --- | --- |
| El monitor resta FPS al juego | Medir con y sin monitor; buffer en memoria; muestreo ajustable |
| Hardware distinto según fabricante | Backends separados; empezar con un solo equipo |
| Anti-cheat frente a capas inyectadas (EAC, BattlEye) | Probar pronto; no inyectar nada propio en el MVP |
| Detectar el juego bajo Proton o Wine | Seguir el árbol de procesos desde Steam o Lutris |
| Diagnósticos erróneos | Hipótesis con confianza y acceso a datos crudos |
| Curva de C++ y alcance excesivo | MVP pequeño, C++ básico moderno, avance por etapas |
| Comparaciones sin validez estadística | Escena repetible, varias corridas, mostrar variación |
| Ideas que se pierden | Este archivo y una bitácora técnica |

---

## 11. Aprendizaje

| Área | Qué recuperar o aprender |
| --- | --- |
| Linux | `/proc`, `/sys`, procesos, permisos, servicios |
| Sistemas operativos | Procesos, hilos, memoria, scheduling |
| Arquitectura | CPU, caché, GPU, memoria, frecuencias |
| Programación | POO, concurrencia, archivos, APIs, C++ |
| Bases de datos | Modelado, consultas, persistencia |
| Estadística | Percentiles, promedios, varianza, anomalías, significancia |
| Visualización | Gráficos temporales e interfaces interactivas |

- Se consulta la documentación (NVIDIA, AMD, Intel, Linux, Vulkan) según la necesidad, sin memorizarla.
- **Bitácora técnica** para descubrimientos, errores, decisiones y aprendizajes. Sirve además como evidencia de proceso.
- El aprendizaje es incremental: la IA es apoyo, no quien construye mientras se observa.

---

## 12. Herramientas existentes de referencia

- **MangoHud:** overlay y logs de FPS y frametime en Linux. Fuente de datos inicial.
- **CoolerControl:** panel en vivo de sensores y control de ventiladores. Referencia visual del árbol de sensores.
- **CapFrameX:** análisis y comparación de rendimiento en Windows.
- **PresentMon:** captura de frametime en Windows.
- **GOverlay, CoreCtrl:** configuración de overlays y control de hardware en Linux.
- Hasta donde se ha visto, las herramientas de Linux se centran en el overlay en tiempo real; el análisis posterior con diagnóstico y comparación con contexto está menos cubierto. Conviene verificarlo antes de afirmarlo ante terceros.

---

## 13. Notas técnicas y correcciones

- FPS y frametime no son lo mismo.
- **vkBasalt** no expone métricas de rendimiento.
- En Linux no existe una API limpia equivalente a PresentMon; en Windows el frametime viene de ETW/PresentMon, no de "Performance Counters" a secas.
- Con el driver propietario de NVIDIA, buena parte de los datos de GPU se leen por **NVML**, no por `hwmon`.
- La lectura de consumo vía RAPL puede requerir permisos elevados según el kernel.
- Una correlación entre métricas no demuestra causalidad.
- No llamar a herramientas externas (por ejemplo, `nvidia-smi`) en bucle dentro del muestreo.

---

## 14. Pendientes

- [x] Repositorio creado
- [x] Collector de CPU, RAM y temperatura con soporte de "N/D" para la temperatura
- [x] Uso de CPU opcional ("N/D" en la primera muestra)
- [ ] Definir el esquema de datos de una sesión (columnas del CSV definidas; faltan los metadatos)
- [x] Clase `CsvSessionWriter` (escribe un CSV con cabecera; `N/D` como campo vacío)
- [ ] Conectar el motor con el escritor para guardar las muestras durante la sesión
- [ ] Collector por proceso (`/proc/PID/status`)
- [ ] Decidir base de datos, frecuencia de muestreo y formato de exportación
- [ ] Definir las primeras reglas de diagnóstico
- [ ] Decidir la fuente de FPS y frametime (captura propia o MangoHud como referencia)
- [ ] Elegir nombre y licencia
- [ ] Probar el comportamiento con anti-cheat

---
