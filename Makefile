TARGET=war
OBJS=main.o display.o game.o particle.o particle_system.o particle_data.o pather.o move.o sprite.o agent.o group.o task.o agent_handler.o tasklist.o data.o data_types.o world.o config.o mapsearchnode.o stationary.o mask.o player.o screen.o config_loader.o attack_data.o attack_type.o building_data.o popup.o gui.o lexgui.o network.o connection.o
# extra flags for non-debug releases: -ffast-math -fopenmp -O3
DEBUGFLAGS=-g -O0
CFLAGS=${DEBUGFLAGS}
CXXFLAGS=${DEBUGFLAGS}

ALLEG=`allegro-config --libs`
BOOST=-lboost_thread-mt


${TARGET}: ${OBJS}
	g++ ${DEBUGFLAGS} -o ${TARGET} ${OBJS} ${ALLEG} ${BOOST}

all: ${TARGET}

clean:
	rm -f ${TARGET} *.o core*
