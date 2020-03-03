# Makefile for game of life

CC_SEQ= pgcc -fast -Minfo=opt
CC_MC= pgcc -fast -ta=multicore -Minfo=mp,par
CC_ACC_M= pgcc -fast -ta=tesla:cc50,managed -Minfo=accel
CC_ACC= pgcc -fast -ta=tesla:cc50 -Minfo=accel

#profiling
PGPROF_RUN=pgprof
PGPROF= pgprof --cpu-profiling-scope instruction

######### sequential
gol_seq: gol_seq.c
	${CC_SEQ} -o gol_seq gol_seq.c

gol_seq_prof: gol_seq
	${PGPROF_RUN} -o gol_seq.prof ./gol_seq
	${PGPROF} -i gol_seq.prof > gol_seq_prof.out

######## multicore
gol_mc: gol_mc.c
	${CC_MC} -ta:multicore -o gol_mc gol_mc.c

gol_mc_prof: gol_mc
	${PGPROF_RUN} -o gol_mc.prof ./gol_mc
	${PGPROF} -i gol_mc.prof

########  acc loops (kind of expect a slowdown like the jacobi example)

gol_acc_loops: gol_acc_loops.c
	${CC_ACC} -ta=tesla:managed -o gol_acc_loops gol_acc_loops.c

gol_acc_loops_prof: gol_acc_loops
	${PGPROF_RUN} -o gol_acc_loops.prof ./gol_acc_loops
	${PGPROF} -i gol_acc_loops.prof

####### acc data

gol_acc_data: gol_acc_data.c
	${CC_ACC} -ta=tesla:cc50 -o gol_acc_data gol_acc_data.c

gol_acc_data_prof: gol_acc_data
	${PGPROF_RUN} -o gol_acc_data.prof ./gol_acc_data
	${PGPROF} -i gol_acc_data.prof

####### still to do: trying different sizes for gang, vector, workers
#######              in a new code file

gol_gvw: gol_gvw.c
	${CC_ACC} -ta=tesla:managed -o gol_gvw gol_gvw.c

gol_gvw_prof: gol_gvw
	${PGPROF_RUN} -o gol_gvw.prof ./gol_gvw
	${PGPROF} -i gol_gvw.prof

####### Combination of data and gang,vector, workers: final.

gol_final: gol_final.c
	${CC_ACC} -ta=tesla:cc50 -o gol_final gol_final.c

gol_final_prof: gol_final
	${PGPROF_RUN} -o gol_final.prof ./gol_final
	${PGPROF} -i gol_final.prof


####### clean
clean:
	rm -f gol_seq gol_mc gol_acc_loops gol_acc_data gol_gvw gol_final
