# CMake configuration for Dobiestation core libraries
set(TARGET Core)
include(DobieHelpers)

# Add all the source files
set(SOURCES
    ee/interpreter/emotionasm.cpp
    ee/interpreter/emotiondisasm.cpp
    ee/interpreter/emotioninterpreter.cpp
    ee/interpreter/emotion_fpu.cpp
    ee/interpreter/emotion_mmi.cpp
    ee/interpreter/emotion_special.cpp
    ee/interpreter/emotion_vu0.cpp
    ee/ipu/chromtable.cpp
    ee/ipu/codedblockpattern.cpp
    ee/ipu/dct_coeff.cpp
    ee/ipu/dct_coeff_table0.cpp
    ee/ipu/dct_coeff_table1.cpp
    ee/ipu/ipu.cpp
    ee/ipu/ipu_fifo.cpp
    ee/ipu/lumtable.cpp
    ee/ipu/mac_addr_inc.cpp
    ee/ipu/mac_b_pic.cpp
    ee/ipu/mac_i_pic.cpp
    ee/ipu/mac_p_pic.cpp
    ee/ipu/motioncode.cpp
    ee/ipu/vlc_table.cpp
    ee/jit/ee_jit.cpp
    ee/jit/ee_jit64.cpp
    ee/jit/ee_jit64_cop2.cpp
    ee/jit/ee_jit64_fpu.cpp
    ee/jit/ee_jit64_fpu_avx.cpp
    ee/jit/ee_jit64_gpr.cpp
    ee/jit/ee_jit64_mmi.cpp
    ee/jit/ee_jittrans.cpp
    ee/vu/vif.cpp
    ee/vu/vu.cpp
    ee/vu/vu_disasm.cpp
    ee/vu/vu_interpreter.cpp
    ee/vu/vu_jit.cpp
    ee/vu/vu_jit64.cpp
    ee/vu/vu_jittrans.cpp
    ee/bios_hle.cpp
    ee/cop0.cpp
    ee/cop1.cpp
    ee/cop2.cpp
    ee/dmac.cpp
    ee/emotion.cpp
    ee/intc.cpp
    ee/timers.cpp
    ee/timers.cpp
    gs/gif.cpp
    gs/gs.cpp
    gs/gscontext.cpp
    gs/gsmem.cpp
    gs/gsregisters.cpp
    gs/gsthread.cpp
    iop/cdvd/bincuereader.cpp
    iop/cdvd/cdvd.cpp
    iop/cdvd/cso_reader.cpp
    iop/cdvd/iso_reader.cpp
    iop/sio2/firewire.cpp
    iop/sio2/gamepad.cpp
    iop/sio2/memcard.cpp
    iop/sio2/sio2.cpp
    iop/spu/spu.cpp
    iop/spu/spu_adpcm.cpp
    iop/spu/spu_envelope.cpp
    iop/spu/spu_interpolate.cpp
    iop/spu/spu_reverb.cpp
    iop/iop.cpp
    iop/cop0.cpp
    iop/dma.cpp
    iop/intc.cpp
    iop/interpreter/iop_interpreter.cpp
    iop/timers.cpp
    jitcommon/emitter64.cpp
    jitcommon/ir_block.cpp
    jitcommon/ir_instr.cpp
    jitcommon/jitcache.cpp
    util/audio.cpp
    util/errors.cpp
    emulator.cpp
    scheduler.cpp
    serialize.cpp
    sif.cpp
)

add_library(${TARGET} ${SOURCES})
set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-g -O0")

# Link all the sources
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Dobie::Core ALIAS ${TARGET})

# Add include directories
target_link_libraries(${TARGET} ${CONAN_LIBS})