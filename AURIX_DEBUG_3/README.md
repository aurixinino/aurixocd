<h1> AURIX™ocd </h1>

<h2>Let's build our own On Chip Debugger application - STEP 3 & 4 </h2>

Here the source codes related to the article: https://aurixocd.blogspot.com/2021/04/lets-build-our-own-on-chip-debugger_21.html and https://aurixocd.blogspot.com/2021/04/lets-build-our-own-on-chip-debugger_27.html

<h3> Tool Interactive User Interface </h3>

See: https://github.com/aurixinino/aurixocd/wiki/Tool-Interactive-User-Interface

<h3> What to do</h3>

Download the sources and compile it by MS Visual Studio 2012 professional or higher. Or, just download the ZIP including the "AurixDebugger.exe" (in the Release directory). You will experience a basic On Chip Debugger DOS application (command line only) for AURIX. Before please ansure to have DAS installed in your PC (see below).

<h4>Syntax</h4>
 * From DOS consolle or Window explorer start AurixDebugger.exe without options. The debugger interactive interface will be started. See: https://github.com/aurixinino/aurixocd/wiki/Tool-Interactive-User-Interface <br>
 * From DOS consolle start AurixDebugger.exe -help. The debugger command line interface help starts. See: https://github.com/aurixinino/aurixocd/wiki/Command-Line-Interface

<h4>Documentation</h4>
See full documentation: https://github.com/aurixinino/aurixocd/wiki

<h3> DAS Installation Guideline </h3>
Please be aware that you need to install DAS before being able to use the AURIX™ocd tool. Download it at https://www.infineon.com/DAS.
Then you must have an AURIX™ board, as available: https://www.infineon.com/cms/en/product/microcontroller/32-bit-tricore-microcontroller/?redirId=41544#!boards

I'm currently using a TC275 Lite Kit: https://www.infineon.com/cms/en/product/evaluation-boards/kit_aurix_tc275_lite/
