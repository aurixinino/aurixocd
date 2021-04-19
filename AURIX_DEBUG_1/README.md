<h1> AURIX™ocd </h1>

<h2>Let's build our own On Chip Debugger application - STEP 1 </h2>

Here the source codes related to the article: https://aurixocd.blogspot.com/2021/04/lets-build-our-own-on-chip-debugger_18.html

<h3> Tool Command Line Interface </h3>
<ul>
  <li>Control core:         run, stop, step <steps> </li>
  <li>Reset (and Halt):     rst, rsthlt </li>
  <li>Read 32 bit word:     read <addr> </li>
  <li>Dump memory:          dump <addr> <n_bytes> </li>
  <li>ASCII viewer:         ascii <addr> <n_bytes> </li>
  <li>Read N bytes:         read <addr> <n_bytes> </li>
  <li>Read 8 bit word:      r8 <addr> </li>
  <li>Seek in memory:       seek <addr> <value> <size> </li>
  <li>Write 32 bit word:    write <addr> <value> </li>
  <li>Write 8 bit word:     w8 <addr> <value> </li>
  <li>Set IP breakpoint:    bpt <addr> </li>
  <li>Clear IP breakpoint:  bpt</li>
  <li>Access HW frequency:  frequ <f_hz></li>
  <li>Exit:                 exit</li>
</ul>

<h3> What to do</h3>

Download the sources and compile it by MS Visual Studio 2012 professional or higher. Or, just download the ZIP including the "Aurixocd.exe" (in the Release directory). You will experience a basic On Chip Debugger DOS application (command line only) for AURIX.

