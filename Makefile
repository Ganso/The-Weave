# Makefile de The Weave (Mega Drive / SGDK)
#
# Se ejecuta DENTRO del contenedor SGDK; lo invoca build-theweave.sh así:
#   docker run ... --entrypoint make <imagen> GDK=/sgdk -f /src/Makefile <target>
#
# POR QUÉ ES AUTOCONTENIDO (y no un simple `include $(GDK)/makefile.gen`):
# el makefile.gen de SGDK cambia entre versiones y eso rompía el build. Las
# versiones nuevas quitaron `-Isrc` de sus includes (y nuestro código incluye
# con rutas relativas a src/: "core/core.h"), renombraron rom_head.c a
# rom_header.c y movieron la salida a out/<tipo>/. Definiendo aquí las reglas
# dependemos solo de `common.mk` (rutas de herramientas, igual en todas las
# versiones) y el proyecto compila con cualquier SGDK, sea el de la imagen
# Docker o uno local más moderno. Mismo enfoque que RedPlanet_MD.
#
# El codegen (tools/gen_*.py) corre en el HOST antes de compilar — lo lanza
# build-theweave.sh — porque la imagen SGDK no incluye python3.
#
# Añadir un .c o un .res NO requiere tocar este archivo: se descubren solos.

GDK ?= /sgdk

include $(GDK)/common.mk

# Compatibilidad entre versiones de SGDK: la carpeta de includes de la librería
# se llamaba INCLUDE_LIB y en las versiones nuevas es INC_LIB. Con esto el
# Makefile vale para las dos (el resto de variables de common.mk no cambian).
INC_LIB ?= $(INCLUDE_LIB)

SRC     := src
RES     := res
INCLUDE := inc
OUT     := out

# --- Descubrimiento de fuentes (hasta 5 niveles: src/spells/player/x.c) -------
# Se excluyen los ficheros de arranque: tienen reglas dedicadas más abajo.
SRC_C  = $(wildcard *.c)
SRC_C += $(wildcard $(SRC)/*.c)
SRC_C += $(wildcard $(SRC)/*/*.c)
SRC_C += $(wildcard $(SRC)/*/*/*.c)
SRC_C += $(wildcard $(SRC)/*/*/*/*.c)
SRC_C := $(filter-out $(SRC)/boot/rom_header.c,$(SRC_C))

SRC_S  = $(wildcard *.s)
SRC_S += $(wildcard $(SRC)/*.s)
SRC_S += $(wildcard $(SRC)/*/*.s)
SRC_S += $(wildcard $(SRC)/*/*/*.s)
SRC_S += $(wildcard $(SRC)/*/*/*/*.s)
SRC_S := $(filter-out $(SRC)/boot/sega.s,$(SRC_S))

SRC_S80  = $(wildcard *.s80)
SRC_S80 += $(wildcard $(SRC)/*.s80)
SRC_S80 += $(wildcard $(SRC)/*/*.s80)

RES_C    = $(wildcard $(RES)/*.c)
RES_S    = $(wildcard $(RES)/*.s)
RES_RES  = $(wildcard *.res)
RES_RES += $(wildcard $(RES)/*.res)

RES_RS   = $(RES_RES:.res=.rs)
RES_H    = $(RES_RES:.res=.h)
RES_DEP  = $(RES_RES:.res=.d)
RES_DEPS = $(addprefix $(OUT)/, $(RES_DEP))

OBJ  = $(RES_RES:.res=.o)
OBJ += $(RES_S:.s=.o)
OBJ += $(RES_C:.c=.o)
OBJ += $(SRC_S80:.s80=.o)
OBJ += $(SRC_S:.s=.o)
OBJ += $(SRC_C:.c=.o)
OBJS := $(addprefix $(OUT)/, $(OBJ))

DEPS := $(OBJS:.o=.d)
LST  := $(SRC_C:.c=.lst)
LSTS := $(addprefix $(OUT)/, $(LST))

# -I$(SRC) es IMPRESCINDIBLE: todo el código incluye por rutas relativas a src/.
# EXTRA_FLAGS queda libre para la línea de órdenes (build-theweave.sh smoke pasa
# ahí -DHACK_SMOKE_BUILD).
INCS := -I. -I$(INCLUDE) -I$(SRC) -I$(RES) -I$(INC_LIB) -I$(RES_LIB)
DEFAULT_FLAGS = $(EXTRA_FLAGS) -DSGDK_GCC -m68000 -fdiagnostics-color=always -Wall -Wextra -Wno-shift-negative-value -Wno-main -Wno-unused-parameter -fno-builtin -ffunction-sections -fdata-sections -fms-extensions $(INCS) -B$(BIN)
FLAGSZ80 := -i$(SRC) -i$(INCLUDE) -i$(RES) -i$(SRC_LIB) -i$(INC_LIB) -i$(INC_LIB)/snd

all: release
default: release

release: FLAGS= $(DEFAULT_FLAGS) -O3 -fuse-linker-plugin -fno-web -fno-gcse -fomit-frame-pointer -flto -flto=auto -ffat-lto-objects
release: CFLAGS= $(FLAGS)
release: AFLAGS= $(FLAGS)
release: LIBMD= $(LIB)/libmd.a
release: $(OUT)/rom.bin $(OUT)/symbol.txt
release: padROM
.PHONY: all default release padROM

debug: FLAGS= $(DEFAULT_FLAGS) -O1 -DDEBUG=1
debug: CFLAGS= $(FLAGS) -ggdb -g
debug: AFLAGS= $(FLAGS)
debug: LIBMD= $(LIB)/libmd_debug.a
debug: $(OUT)/rom.bin $(OUT)/rom.out $(OUT)/symbol.txt
debug: padROM
.PHONY: debug

-include $(DEPS)

# --- Limpieza -----------------------------------------------------------------
clean:
	$(RM) -f $(OBJS) $(OUT)/sega.o $(OUT)/sega.s $(OUT)/rom_header.bin $(OUT)/rom_head.bin $(OUT)/rom_header.o $(OUT)/rom.out
	$(RM) -f $(RES_RS) $(RES_H) $(RES_DEP) $(RES_DEPS) $(DEPS) $(LSTS)
	$(RM) -f $(OUT)/out.lst $(OUT)/cmd_ $(OUT)/symbol.txt $(OUT)/rom.nm $(OUT)/rom.wch $(OUT)/rom.bin
.PHONY: clean

# --- ROM ----------------------------------------------------------------------
padROM: $(OUT)/rom.bin
	$(SIZEBND) $(OUT)/rom.bin -sizealign 131072 -checksum

$(OUT)/rom.bin: $(OUT)/rom.out $(OUT)/symbol.txt
	$(OBJCPY) -O binary $(OUT)/rom.out $(OUT)/rom.bin

$(OUT)/symbol.txt: $(OUT)/rom.out
	$(NM) $(LTO_PLUGIN) -nl $(OUT)/rom.out > $(OUT)/symbol.txt

$(OUT)/rom.out: $(OUT)/sega.o $(OUT)/cmd_ $(LIBMD)
	$(MKDIR) -p $(dir $@)
	$(CC) -m68000 -B$(BIN) -n -T $(GDK)/md.ld -nostdlib $(OUT)/sega.o @$(OUT)/cmd_ $(LIBMD) $(LIBGCC) -o $(OUT)/rom.out -Wl,--gc-sections -flto -flto=auto -ffat-lto-objects
	$(RM) $(OUT)/cmd_

$(OUT)/cmd_: $(OBJS)
	$(MKDIR) -p $(dir $@)
	$(ECHO) "$(OBJS)" > $(OUT)/cmd_

# --- Arranque -----------------------------------------------------------------
# El sega.s lo aporta SGDK (así no arrastramos una copia que se queda vieja);
# la cabecera de la ROM (título, autor, región) es NUESTRA: src/boot/rom_header.c.
# NOTA: sega.s referencia out/rom_header.bin por ese nombre exacto.
$(OUT)/sega.o: $(OUT)/rom_header.bin
	$(MKDIR) -p $(dir $@)
	$(CP) $(SRC_LIB)/boot/sega.s $(OUT)/sega.s
	$(CC) -x assembler-with-cpp -Wa,--register-prefix-optional,--bitwise-or $(AFLAGS) -c $(OUT)/sega.s -o $@

# El sega.s de SGDK incrusta la cabecera por NOMBRE DE FICHERO, y ese nombre
# cambió entre versiones (rom_head.bin -> rom_header.bin). Generamos los dos
# para que sirva cualquiera sin tener que detectar la versión.
$(OUT)/rom_header.bin: $(OUT)/rom_header.o
	$(OBJCPY) -O binary $< $@
	$(CP) $@ $(OUT)/rom_head.bin

$(OUT)/rom_header.o: $(SRC)/boot/rom_header.c
	$(MKDIR) -p $(dir $@)
	$(CC) $(DEFAULT_FLAGS) -c $< -o $@

# --- Reglas genéricas ---------------------------------------------------------
$(OUT)/%.lst: %.c
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/%.o: %.c
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(OUT)/%.o: %.s
	$(MKDIR) -p $(dir $@)
	$(CC) -x assembler-with-cpp -Wa,--register-prefix-optional,--bitwise-or $(AFLAGS) -MMD -c $< -o $@

$(OUT)/%.o: %.rs
	$(MKDIR) -p $(dir $@)
	$(CC) -x assembler-with-cpp -Wa,--register-prefix-optional,--bitwise-or $(AFLAGS) -c $*.rs -o $@
	$(CP) $*.d $(OUT)/$*.d
	$(RM) $*.d

%.rs: %.res
	$(RESCOMP) $*.res $*.rs -dep $(OUT)/$*.o

%.o80: %.s80
	$(ASMZ80) $(FLAGSZ80) $< $@ $(OUT)/out.lst

%.s: %.o80
	$(BINTOS) $<
