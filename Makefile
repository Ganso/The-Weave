# Makefile de The Weave (Mega Drive / SGDK)
#
# Se ejecuta DENTRO del contenedor SGDK; lo invoca build-theweave.sh así:
#   docker run ... --entrypoint make <imagen> GDK=/sgdk -f /src/Makefile <target>
#
# Envuelve el makefile.gen de SGDK, que ya hace wildcard discovery de
# src/*.c, src/*/*.c y res/*.res (añadir un .c NO requiere tocar este archivo)
# y aporta los targets release / debug / clean con -Isrc -Ires.
#
# El codegen (tools/gen_*.py) corre en el HOST antes de compilar — lo lanza
# build-theweave.sh — porque la imagen SGDK no incluye python3.

GDK ?= /sgdk

include $(GDK)/makefile.gen

# --- Targets propios del repo -------------------------------------------------
# (Fase 7 añadirá aquí el target `smoke` para la ROM de smoke test)
