#=======================================================================
# Copyright © 2019-2024 Pedro López-Cabanillas <plcl@users.sf.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Kitware, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=======================================================================

if (NOT TARGET Qt${QT_VERSION_MAJOR}::lconvert)
    message(FATAL_ERROR "The package \"Qt${QT_VERSION_MAJOR}LinguistTools\" is required.")
endif()

set(Qt_LCONVERT_EXECUTABLE Qt${QT_VERSION_MAJOR}::lconvert)

function(ADD_APP_TRANSLATIONS_RESOURCE res_file)
    set(_qm_files ${ARGN})
    set(_res_file ${CMAKE_CURRENT_BINARY_DIR}/app_translations.qrc)

    file(WRITE ${_res_file} "<!DOCTYPE RCC><RCC version=\"1.0\">\n <qresource prefix=\"/\">\n")
    foreach(_lang ${_qm_files})
        get_filename_component(_filename ${_lang} NAME)
        file(APPEND ${_res_file} "  <file>${_filename}</file>\n")
    endforeach()
    file(APPEND ${_res_file} " </qresource>\n</RCC>\n")
    set(${res_file} ${_res_file} PARENT_SCOPE)
endfunction()

function(ADD_QT_TRANSLATIONS_RESOURCE res_file)
    set(_languages ${ARGN})
    set(_res_file ${CMAKE_CURRENT_BINARY_DIR}/qt_translations.qrc)
    set(_patterns qtbase qtmultimedia qtscript qtxmlpatterns)
    get_filename_component(_srcdir "${Qt${QT_VERSION_MAJOR}_DIR}/../../../translations" ABSOLUTE)
    set(_outfiles)
    foreach(_lang ${_languages})
        set(_infiles)
        set(_out qt_${_lang}.qm)
        foreach(_pat ${_patterns})
            set(_file "${_srcdir}/${_pat}_${_lang}.qm")
            if (EXISTS ${_file})
                list(APPEND _infiles ${_file})
            endif()
        endforeach()
        if(_infiles)
            add_custom_command(OUTPUT ${_out}
                COMMAND ${Qt_LCONVERT_EXECUTABLE}
                ARGS -i ${_infiles} -o ${_out}
                COMMAND_EXPAND_LISTS VERBATIM)
            list(APPEND _outfiles ${_out})
            set_property(SOURCE ${_out} PROPERTY SKIP_AUTOGEN ON)
        endif()
    endforeach()
    file(WRITE ${_res_file} "<!DOCTYPE RCC><RCC version=\"1.0\">\n <qresource prefix=\"/\">\n")
    foreach(_file ${_outfiles})
        get_filename_component(_filename ${_file} NAME)
        file(APPEND ${_res_file} "  <file>${_filename}</file>\n")
    endforeach()
    file(APPEND ${_res_file} " </qresource>\n</RCC>\n")
    set(${res_file} ${_res_file} PARENT_SCOPE)
endfunction()

function(ADD_QT_TRANSLATIONS out_files)
    set(_languages ${ARGN})
    set(_patterns qtbase qtmultimedia qtscript qtxmlpatterns)
    get_filename_component(_srcdir "${Qt${QT_VERSION_MAJOR}_DIR}/../../../translations" ABSOLUTE)
    set(_outfiles)
    foreach(_lang ${_languages})
        set(_infiles)
        set(_out qt_${_lang}.qm)
        foreach(_pat ${_patterns})
            set(_file "${_srcdir}/${_pat}_${_lang}.qm")
            if (EXISTS ${_file})
                list(APPEND _infiles ${_file})
            endif()
        endforeach()
        if(_infiles)
            add_custom_command(OUTPUT ${_out}
                COMMAND ${Qt_LCONVERT_EXECUTABLE}
                ARGS -i ${_infiles} -o ${_out}
                COMMAND_EXPAND_LISTS VERBATIM)
            list(APPEND _outfiles ${_out})
            set_property(SOURCE ${_out} PROPERTY SKIP_AUTOGEN ON)
        endif()
    endforeach()
    set(${out_files} ${_outfiles} PARENT_SCOPE)
endfunction()
