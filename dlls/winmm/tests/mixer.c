/*
 * Test mixer
 *
 * Copyright (c) 2004 Robert Reif
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * To Do:
 * examine and update control details
 * add interactive tests
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NONAMELESSSTRUCT
#define NONAMELESSUNION
#include "wine/test.h"
#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "mmsystem.h"
#include "mmddk.h"

#include "winmm_test.h"

static const char * line_flags(DWORD fdwLine)
{
    static char flags[100];
    BOOL first=TRUE;
    flags[0]=0;
    if (fdwLine&MIXERLINE_LINEF_ACTIVE) {
        strcat(flags,"MIXERLINE_LINEF_ACTIVE");
        first=FALSE;
    }
    if (fdwLine&MIXERLINE_LINEF_DISCONNECTED) {
        if (!first)
            strcat(flags, "|");

        strcat(flags,"MIXERLINE_LINEF_DISCONNECTED");
        first=FALSE;
    }

    if (fdwLine&MIXERLINE_LINEF_SOURCE) {
        if (!first)
            strcat(flags, "|");

        strcat(flags,"MIXERLINE_LINEF_SOURCE");
    }

    return flags;
}

static const char * component_type(DWORD dwComponentType)
{
#define TYPE_TO_STR(x) case x: return #x
    switch (dwComponentType) {
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_UNDEFINED);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_DIGITAL);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_LINE);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_MONITOR);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_HEADPHONES);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_TELEPHONE);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_DST_VOICEIN);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_DIGITAL);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_LINE);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY);
    TYPE_TO_STR(MIXERLINE_COMPONENTTYPE_SRC_ANALOG);
    }
#undef TYPE_TO_STR
    return "UNKNOWN";
}

static const char * target_type(DWORD dwType)
{
#define TYPE_TO_STR(x) case x: return #x
    switch (dwType) {
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_UNDEFINED);
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_WAVEOUT);
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_WAVEIN);
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_MIDIOUT);
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_MIDIIN);
    TYPE_TO_STR(MIXERLINE_TARGETTYPE_AUX);
    }
#undef TYPE_TO_STR
    return "UNKNOWN";
}

static const char * control_type(DWORD dwControlType)
{
#define TYPE_TO_STR(x) case x: return #x
    switch (dwControlType) {
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_CUSTOM);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_BOOLEANMETER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_SIGNEDMETER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_PEAKMETER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_BOOLEAN);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_ONOFF);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MUTE);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MONO);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_LOUDNESS);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_STEREOENH);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_BASS_BOOST);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_BUTTON);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_DECIBELS);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_SIGNED);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_UNSIGNED);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_PERCENT);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_SLIDER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_PAN);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_QSOUNDPAN);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_FADER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_VOLUME);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_BASS);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_TREBLE);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_EQUALIZER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_SINGLESELECT);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MUX);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MIXER);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MICROTIME);
    TYPE_TO_STR(MIXERCONTROL_CONTROLTYPE_MILLITIME);
    }
#undef TYPE_TO_STR
    return "UNKNOWN";
}

static const char * control_flags(DWORD fdwControl)
{
    static char flags[100];
    BOOL first=TRUE;
    flags[0]=0;
    if (fdwControl&MIXERCONTROL_CONTROLF_UNIFORM) {
        strcat(flags,"MIXERCONTROL_CONTROLF_UNIFORM");
        first=FALSE;
    }
    if (fdwControl&MIXERCONTROL_CONTROLF_MULTIPLE) {
        if (!first)
            strcat(flags, "|");

        strcat(flags,"MIXERCONTROL_CONTROLF_MULTIPLE");
        first=FALSE;
    }

    if (fdwControl&MIXERCONTROL_CONTROLF_DISABLED) {
        if (!first)
            strcat(flags, "|");

        strcat(flags,"MIXERCONTROL_CONTROLF_DISABLED");
    }

    return flags;
}
void mixer_test_deviceA(int device)
{
    MIXERCAPSA capsA;
    HMIXER mix;
    MMRESULT rc;
    DWORD d,s,ns,nc;

    rc=mixerGetDevCapsA(device,0,sizeof(capsA));
    ok(rc==MMSYSERR_INVALPARAM,
       "mixerGetDevCapsA: MMSYSERR_INVALPARAM expected, got %s\n",
       mmsys_error(rc));

    rc=mixerGetDevCapsA(device,&capsA,4);
    ok(rc==MMSYSERR_NOERROR,
       "mixerGetDevCapsA: MMSYSERR_NOERROR expected, got %s\n",
       mmsys_error(rc));

    rc=mixerGetDevCapsA(device,&capsA,sizeof(capsA));
    ok(rc==MMSYSERR_NOERROR,
       "mixerGetDevCapsA: MMSYSERR_NOERROR expected, got %s\n",
       mmsys_error(rc));

    trace("  %d: \"%s\" %d.%d (%d:%d) destinations=%ld\n", device,
          capsA.szPname, capsA.vDriverVersion >> 8,
          capsA.vDriverVersion & 0xff,capsA.wMid,capsA.wPid,
          capsA.cDestinations);
    rc=mixerOpen(&mix, device, 0, 0, 0);
    ok(rc==MMSYSERR_NOERROR,
       "mixerOpen: MMSYSERR_BADDEVICEID expected, got %s\n",mmsys_error(rc));
    if (rc==MMSYSERR_NOERROR) {
        for (d=0;d<capsA.cDestinations;d++) {
            MIXERLINEA mixerlineA;
            mixerlineA.cbStruct = 0;
            mixerlineA.dwDestination=d;
            rc=mixerGetLineInfoA((HMIXEROBJ)mix,&mixerlineA,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoA(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineA.cbStruct = sizeof(mixerlineA);
            mixerlineA.dwDestination=capsA.cDestinations;
            rc=mixerGetLineInfoA((HMIXEROBJ)mix,&mixerlineA,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoA(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineA.cbStruct = sizeof(mixerlineA);
            mixerlineA.dwDestination=d;
            rc=mixerGetLineInfoA((HMIXEROBJ)mix,0,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoA(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineA.cbStruct = sizeof(mixerlineA);
            mixerlineA.dwDestination=d;
            rc=mixerGetLineInfoA((HMIXEROBJ)mix,&mixerlineA,-1);
            ok(rc==MMSYSERR_INVALFLAG,
               "mixerGetLineInfoA(-1): MMSYSERR_INVALFLAG expected, got %s\n",
               mmsys_error(rc));

            mixerlineA.cbStruct = sizeof(mixerlineA);
            mixerlineA.dwDestination=d;
            rc=mixerGetLineInfoA((HMIXEROBJ)mix,&mixerlineA,
                                  MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_NOERROR,
               "mixerGetLineInfoA(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_NOERROR expected, got %s\n",
               mmsys_error(rc));
            if (rc==MMSYSERR_NOERROR) {
                trace("    %ld: \"%s\" (%s) Destination=%ld Source=%ld\n",
                      d,mixerlineA.szShortName, mixerlineA.szName,
                      mixerlineA.dwDestination,mixerlineA.dwSource);
                trace("        LineID=%08lx Channels=%ld "
                      "Connections=%ld Controls=%ld \n",
                      mixerlineA.dwLineID,mixerlineA.cChannels,
                      mixerlineA.cConnections,mixerlineA.cControls);
                trace("        State=0x%08lx(%s)\n",
                      mixerlineA.fdwLine,line_flags(mixerlineA.fdwLine));
                trace("        ComponentType=%s\n",
                      component_type(mixerlineA.dwComponentType));
                trace("        Type=%s\n",
                      target_type(mixerlineA.Target.dwType));
                trace("        Device=%ld (%s) %d.%d (%d:%d)\n",
                      mixerlineA.Target.dwDeviceID,
                      mixerlineA.Target.szPname,
                      mixerlineA.Target.vDriverVersion >> 8,
                      mixerlineA.Target.vDriverVersion & 0xff,
                      mixerlineA.Target.wMid, mixerlineA.Target.wPid);
            }
            ns=mixerlineA.cConnections;
            for(s=0;s<ns;s++) {
                mixerlineA.cbStruct = sizeof(mixerlineA);
                mixerlineA.dwDestination=d;
                mixerlineA.dwSource=s;
                rc=mixerGetLineInfoA((HMIXEROBJ)mix,&mixerlineA,
                                     MIXER_GETLINEINFOF_SOURCE);
                ok(rc==MMSYSERR_NOERROR,
                   "mixerGetLineInfoA(MIXER_GETLINEINFOF_SOURCE): "
                   "MMSYSERR_NOERROR expected, got %s\n",
                   mmsys_error(rc));
                if (rc==MMSYSERR_NOERROR) {
                    LPMIXERCONTROLA    array;
                    MIXERLINECONTROLSA controls;
                    trace("      %ld: \"%s\" (%s) Destination=%ld Source=%ld\n",
                          s,mixerlineA.szShortName, mixerlineA.szName,
                          mixerlineA.dwDestination,mixerlineA.dwSource);
                    trace("          LineID=%08lx Channels=%ld "
                          "Connections=%ld Controls=%ld \n",
                          mixerlineA.dwLineID,mixerlineA.cChannels,
                          mixerlineA.cConnections,mixerlineA.cControls);
                    trace("          State=0x%08lx(%s)\n",
                          mixerlineA.fdwLine,line_flags(mixerlineA.fdwLine));
                    trace("          ComponentType=%s\n",
                          component_type(mixerlineA.dwComponentType));
                    trace("          Type=%s\n",
                          target_type(mixerlineA.Target.dwType));
                    trace("          Device=%ld (%s) %d.%d (%d:%d)\n",
                          mixerlineA.Target.dwDeviceID,
                          mixerlineA.Target.szPname,
                          mixerlineA.Target.vDriverVersion >> 8,
                          mixerlineA.Target.vDriverVersion & 0xff,
                          mixerlineA.Target.wMid, mixerlineA.Target.wPid);
                    if (mixerlineA.cControls) {
                        array=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,
                            mixerlineA.cControls*sizeof(MIXERCONTROLA));
                        if (array) {
                            rc=mixerGetLineControlsA((HMIXEROBJ)mix,0,
                                                      MIXER_GETLINECONTROLSF_ALL);
                            ok(rc==MMSYSERR_INVALPARAM,
                               "mixerGetLineControlsA(MIXER_GETLINECONTROLSF_ALL): "
                               "MMSYSERR_INVALPARAM expected, got %s\n",
                               mmsys_error(rc));

                            rc=mixerGetLineControlsA((HMIXEROBJ)mix,&controls,-1);
                            ok(rc==MMSYSERR_INVALFLAG||rc==MMSYSERR_INVALPARAM,
                               "mixerGetLineControlsA(-1): "
                               "MMSYSERR_INVALFLAG or MMSYSERR_INVALPARAM expected, got %s\n",
                               mmsys_error(rc));

                            controls.cbStruct = sizeof(MIXERLINECONTROLSA);
                            controls.cControls = mixerlineA.cControls;
                            controls.dwLineID = mixerlineA.dwLineID;
                            controls.pamxctrl = array;
                            controls.cbmxctrl = sizeof(MIXERCONTROLA);

                            /* FIXME: do MIXER_GETLINECONTROLSF_ONEBYID
                             * and MIXER_GETLINECONTROLSF_ONEBYTYPE
                             */
                            rc=mixerGetLineControlsA((HMIXEROBJ)mix,&controls,
                                                     MIXER_GETLINECONTROLSF_ALL);
                            ok(rc==MMSYSERR_NOERROR,
                               "mixerGetLineControlsA(MIXER_GETLINECONTROLSF_ALL): "
                               "MMSYSERR_NOERROR expected, got %s\n",
                               mmsys_error(rc));
                            if (rc==MMSYSERR_NOERROR) {
                                for(nc=0;nc<mixerlineA.cControls;nc++) {
                                    trace("        %ld: \"%s\" (%s) ControlID=%ld\n", nc,
                                          array[nc].szShortName,
                                          array[nc].szName, array[nc].dwControlID);
                                    trace("            ControlType=%s\n",
                                           control_type(array[nc].dwControlType));
                                    trace("            Control=0x%08lx(%s)\n",
                                          array[nc].fdwControl,
                                          control_flags(array[nc].fdwControl));
                                    trace("            Items=%ld Min=%ld Max=%ld Step=%ld\n",
                                          array[nc].cMultipleItems,
                                          array[nc].Bounds.s1.dwMinimum,
                                          array[nc].Bounds.s1.dwMaximum,
                                          array[nc].Metrics.cSteps);
                                }
                            }

                            HeapFree(GetProcessHeap(),0,array);
                        }
                    }
                }
            }
        }
        rc=mixerClose(mix);
        ok(rc==MMSYSERR_NOERROR,
           "mixerClose: MMSYSERR_BADDEVICEID expected, got %s\n",
           mmsys_error(rc));
    }
}

void mixer_test_deviceW(int device)
{
    MIXERCAPSW capsW;
    HMIXER mix;
    MMRESULT rc;
    DWORD d,s,ns,nc;
    char szShortName[MIXER_SHORT_NAME_CHARS];
    char szName[MIXER_LONG_NAME_CHARS];
    char szPname[MAXPNAMELEN];

    rc=mixerGetDevCapsW(device,0,sizeof(capsW));
    ok(rc==MMSYSERR_INVALPARAM,
       "mixerGetDevCapsW: MMSYSERR_INVALPARAM expected, got %s\n",
       mmsys_error(rc));

    rc=mixerGetDevCapsW(device,&capsW,4);
    ok(rc==MMSYSERR_NOERROR,
       "mixerGetDevCapsW: MMSYSERR_NOERROR expected, got %s\n",
       mmsys_error(rc));

    rc=mixerGetDevCapsW(device,&capsW,sizeof(capsW));
    ok(rc==MMSYSERR_NOERROR,
       "mixerGetDevCapsW: MMSYSERR_NOERROR expected, got %s\n",
       mmsys_error(rc));

    WideCharToMultiByte(CP_ACP,0,capsW.szPname, MAXPNAMELEN,szPname,
                        MAXPNAMELEN,NULL,NULL);
    trace("  %d: \"%s\" %d.%d (%d:%d) destinations=%ld\n", device,
          szPname, capsW.vDriverVersion >> 8,
          capsW.vDriverVersion & 0xff,capsW.wMid,capsW.wPid,
          capsW.cDestinations);
    rc=mixerOpen(&mix, device, 0, 0, 0);
    ok(rc==MMSYSERR_NOERROR,
       "mixerOpen: MMSYSERR_BADDEVICEID expected, got %s\n",mmsys_error(rc));
    if (rc==MMSYSERR_NOERROR) {
        for (d=0;d<capsW.cDestinations;d++) {
            MIXERLINEW mixerlineW;
            mixerlineW.cbStruct = 0;
            mixerlineW.dwDestination=d;
            rc=mixerGetLineInfoW((HMIXEROBJ)mix,&mixerlineW,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoW(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineW.cbStruct = sizeof(mixerlineW);
            mixerlineW.dwDestination=capsW.cDestinations;
            rc=mixerGetLineInfoW((HMIXEROBJ)mix,&mixerlineW,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoW(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineW.cbStruct = sizeof(mixerlineW);
            mixerlineW.dwDestination=d;
            rc=mixerGetLineInfoW((HMIXEROBJ)mix,0,
                                 MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_INVALPARAM,
               "mixerGetLineInfoW(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_INVALPARAM expected, got %s\n",
               mmsys_error(rc));

            mixerlineW.cbStruct = sizeof(mixerlineW);
            mixerlineW.dwDestination=d;
            rc=mixerGetLineInfoW((HMIXEROBJ)mix,&mixerlineW,-1);
            ok(rc==MMSYSERR_INVALFLAG,
               "mixerGetLineInfoW(-1): MMSYSERR_INVALFLAG expected, got %s\n",
               mmsys_error(rc));

            mixerlineW.cbStruct = sizeof(mixerlineW);
            mixerlineW.dwDestination=d;
            rc=mixerGetLineInfoW((HMIXEROBJ)mix,&mixerlineW,
                                  MIXER_GETLINEINFOF_DESTINATION);
            ok(rc==MMSYSERR_NOERROR,
               "mixerGetLineInfoW(MIXER_GETLINEINFOF_DESTINATION): "
               "MMSYSERR_NOERROR expected, got %s\n",
               mmsys_error(rc));
            if (rc==MMSYSERR_NOERROR) {
                WideCharToMultiByte(CP_ACP,0,mixerlineW.szShortName,
                    MIXER_SHORT_NAME_CHARS,szShortName,
                    MIXER_SHORT_NAME_CHARS,NULL,NULL);
                WideCharToMultiByte(CP_ACP,0,mixerlineW.szName,
                    MIXER_LONG_NAME_CHARS,szName,
                    MIXER_LONG_NAME_CHARS,NULL,NULL);
                WideCharToMultiByte(CP_ACP,0,mixerlineW.Target.szPname,
                    MAXPNAMELEN,szPname,
                    MAXPNAMELEN,NULL, NULL);
                trace("    %ld: \"%s\" (%s) Destination=%ld Source=%ld\n",
                      d,szShortName,szName,
                      mixerlineW.dwDestination,mixerlineW.dwSource);
                trace("        LineID=%08lx Channels=%ld "
                      "Connections=%ld Controls=%ld \n",
                      mixerlineW.dwLineID,mixerlineW.cChannels,
                      mixerlineW.cConnections,mixerlineW.cControls);
                trace("        State=0x%08lx(%s)\n",
                      mixerlineW.fdwLine,line_flags(mixerlineW.fdwLine));
                trace("        ComponentType=%s\n",
                      component_type(mixerlineW.dwComponentType));
                trace("        Type=%s\n",
                      target_type(mixerlineW.Target.dwType));
                trace("        Device=%ld (%s) %d.%d (%d:%d)\n",
                      mixerlineW.Target.dwDeviceID,szPname,
                      mixerlineW.Target.vDriverVersion >> 8,
                      mixerlineW.Target.vDriverVersion & 0xff,
                      mixerlineW.Target.wMid, mixerlineW.Target.wPid);
            }
            ns=mixerlineW.cConnections;
            for(s=0;s<ns;s++) {
                mixerlineW.cbStruct = sizeof(mixerlineW);
                mixerlineW.dwDestination=d;
                mixerlineW.dwSource=s;
                rc=mixerGetLineInfoW((HMIXEROBJ)mix,&mixerlineW,
                                     MIXER_GETLINEINFOF_SOURCE);
                ok(rc==MMSYSERR_NOERROR,
                   "mixerGetLineInfoW(MIXER_GETLINEINFOF_SOURCE): "
                   "MMSYSERR_NOERROR expected, got %s\n",
                   mmsys_error(rc));
                if (rc==MMSYSERR_NOERROR) {
                    LPMIXERCONTROLW    array;
                    MIXERLINECONTROLSW controls;
                    WideCharToMultiByte(CP_ACP,0,mixerlineW.szShortName,
                        MIXER_SHORT_NAME_CHARS,szShortName,
                        MIXER_SHORT_NAME_CHARS,NULL,NULL);
                    WideCharToMultiByte(CP_ACP,0,mixerlineW.szName,
                        MIXER_LONG_NAME_CHARS,szName,
                        MIXER_LONG_NAME_CHARS,NULL,NULL);
                    WideCharToMultiByte(CP_ACP,0,mixerlineW.Target.szPname,
                        MAXPNAMELEN,szPname,
                        MAXPNAMELEN,NULL, NULL);
                    trace("      %ld: \"%s\" (%s) Destination=%ld Source=%ld\n",
                          s,szShortName,szName,
                          mixerlineW.dwDestination,mixerlineW.dwSource);
                    trace("          LineID=%08lx Channels=%ld "
                          "Connections=%ld Controls=%ld \n",
                          mixerlineW.dwLineID,mixerlineW.cChannels,
                          mixerlineW.cConnections,mixerlineW.cControls);
                    trace("          State=0x%08lx(%s)\n",
                          mixerlineW.fdwLine,line_flags(mixerlineW.fdwLine));
                    trace("          ComponentType=%s\n",
                          component_type(mixerlineW.dwComponentType));
                    trace("          Type=%s\n",
                          target_type(mixerlineW.Target.dwType));
                    trace("          Device=%ld (%s) %d.%d (%d:%d)\n",
                          mixerlineW.Target.dwDeviceID,szPname,
                          mixerlineW.Target.vDriverVersion >> 8,
                          mixerlineW.Target.vDriverVersion & 0xff,
                          mixerlineW.Target.wMid, mixerlineW.Target.wPid);
                    if (mixerlineW.cControls) {
                        array=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,
                            mixerlineW.cControls*sizeof(MIXERCONTROLW));
                        if (array) {
                            rc=mixerGetLineControlsW((HMIXEROBJ)mix,0,
                                                     MIXER_GETLINECONTROLSF_ALL);
                            ok(rc==MMSYSERR_INVALPARAM,
                               "mixerGetLineControlsW(MIXER_GETLINECONTROLSF_ALL): "
                               "MMSYSERR_INVALPARAM expected, got %s\n",
                               mmsys_error(rc));
                            rc=mixerGetLineControlsW((HMIXEROBJ)mix,&controls,
                                                     -1);
                            ok(rc==MMSYSERR_INVALFLAG||rc==MMSYSERR_INVALPARAM,
                               "mixerGetLineControlsA(-1): "
                               "MMSYSERR_INVALFLAG or MMSYSERR_INVALPARAM expected, got %s\n",
                               mmsys_error(rc));

                            controls.cbStruct = sizeof(MIXERLINECONTROLSW);
                            controls.cControls = mixerlineW.cControls;
                            controls.dwLineID = mixerlineW.dwLineID;
                            controls.pamxctrl = array;
                            controls.cbmxctrl = sizeof(MIXERCONTROLW);

                            /* FIXME: do MIXER_GETLINECONTROLSF_ONEBYID
                             * and MIXER_GETLINECONTROLSF_ONEBYTYPE
                             */
                            rc=mixerGetLineControlsW((HMIXEROBJ)mix,&controls,
                                                     MIXER_GETLINECONTROLSF_ALL);
                            ok(rc==MMSYSERR_NOERROR,
                               "mixerGetLineControlsW(MIXER_GETLINECONTROLSF_ALL): "
                               "MMSYSERR_NOERROR expected, got %s\n",
                               mmsys_error(rc));
                            if (rc==MMSYSERR_NOERROR) {
                                for(nc=0;nc<mixerlineW.cControls;nc++) {
                                    WideCharToMultiByte(CP_ACP,0,array[nc].szShortName,
                                        MIXER_SHORT_NAME_CHARS,szShortName,
                                        MIXER_SHORT_NAME_CHARS,NULL,NULL);
                                    WideCharToMultiByte(CP_ACP,0,array[nc].szName,
                                        MIXER_LONG_NAME_CHARS,szName,
                                        MIXER_LONG_NAME_CHARS,NULL,NULL);
                                    trace("        %ld: \"%s\" (%s) ControlID=%ld\n", nc,
                                          szShortName, szName, array[nc].dwControlID);
                                    trace("            ControlType=%s\n",
                                           control_type(array[nc].dwControlType));
                                    trace("            Control=0x%08lx(%s)\n",
                                          array[nc].fdwControl,
                                          control_flags(array[nc].fdwControl));
                                    trace("            Items=%ld Min=%ld Max=%ld Step=%ld\n",
                                          array[nc].cMultipleItems,
                                          array[nc].Bounds.s1.dwMinimum,
                                          array[nc].Bounds.s1.dwMaximum,
                                          array[nc].Metrics.cSteps);
                                }
                            }

                            HeapFree(GetProcessHeap(),0,array);
                        }
                    }
                }
            }
        }
        rc=mixerClose(mix);
        ok(rc==MMSYSERR_NOERROR,
           "mixerClose: MMSYSERR_BADDEVICEID expected, got %s\n",
           mmsys_error(rc));
    }
}

void mixer_testsA()
{
    MIXERCAPSA capsA;
    HMIXER mix;
    MMRESULT rc;
    UINT ndev, d;

    trace("--- Testing ASCII functions ---\n");

    ndev=mixerGetNumDevs();
    trace("found %d Mixer devices\n",ndev);

    rc=mixerGetDevCapsA(ndev+1,&capsA,sizeof(capsA));
    ok(rc==MMSYSERR_BADDEVICEID,
       "mixerGetDevCapsA: MMSYSERR_BADDEVICEID expected, got %s\n",
       mmsys_error(rc));

    rc=mixerOpen(&mix, ndev+1, 0, 0, 0);
    ok(rc==MMSYSERR_BADDEVICEID,
       "mixerOpen: MMSYSERR_BADDEVICEID expected, got %s\n",
       mmsys_error(rc));

    for (d=0;d<ndev;d++)
        mixer_test_deviceA(d);
}

void mixer_testsW()
{
    MIXERCAPSW capsW;
    HMIXER mix;
    MMRESULT rc;
    UINT ndev, d;

    trace("--- Testing WCHAR functions ---\n");

    ndev=mixerGetNumDevs();
    trace("found %d Mixer devices\n",ndev);

    rc=mixerGetDevCapsW(ndev+1,&capsW,sizeof(capsW));
    ok(rc==MMSYSERR_BADDEVICEID,
       "mixerGetDevCapsW: MMSYSERR_BADDEVICEID expected, got %s\n",
       mmsys_error(rc));

    rc=mixerOpen(&mix, ndev+1, 0, 0, 0);
    ok(rc==MMSYSERR_BADDEVICEID,
       "mixerOpen: MMSYSERR_BADDEVICEID expected, got %s\n",
       mmsys_error(rc));

    for (d=0;d<ndev;d++)
        mixer_test_deviceW(d);
}

START_TEST(mixer)
{
    mixer_testsA();
    mixer_testsW();
}
