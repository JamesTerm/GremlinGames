<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="9.00"
	Name="Plugins net"
	SccProjectName=""
	SccLocalPath=""
	Keyword="Win32Proj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="Debug"
			IntermediateDirectory="osgdb_net.dir\Debug"
			ConfigurationType="2"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="FALSE"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalOptions=" /Zm1000"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				BasicRuntimeChecks="1"
				CompileAs="2"
				DebugInformationFormat="3"
				ExceptionHandling="1"
				InlineFunctionExpansion="0"
				Optimization="0"
				RuntimeLibrary="3"
				RuntimeTypeInfo="TRUE"
				WarningLevel="3"
				PreprocessorDefinitions="WIN32,_WINDOWS,_DEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;,osgdb_net_EXPORTS"
				AssemblerListingLocation="Debug"
				ObjectFile="$(IntDir)\"
				ProgramDataBaseFileName="$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/Debug/osgPlugins-2.5.0/osgdb_netd.pdb"
/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCResourceCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				PreprocessorDefinitions="WIN32,_WINDOWS,_DEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;,osgdb_net_EXPORTS"/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,_DEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;,osgdb_net_EXPORTS"
				MkTypLibCompatible="FALSE"
				TargetEnvironment="1"
				GenerateStublessProxies="TRUE"
				TypeLibraryName="$(InputName).tlb"
				OutputDirectory="$(IntDir)"
				HeaderFileName="$(InputName).h"
				DLLDataFileName=""
				InterfaceIdentifierFileName="$(InputName)_i.c"
				ProxyFileName="$(InputName)_p.c"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCPostBuildEventTool"/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions=" /STACK:10000000 /machine:I386 /debug"
				AdditionalDependencies="$(NOINHERIT) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OpenThreadsd.lib osgd.lib osgDBd.lib osgUtild.lib wsock32.lib osgd.lib OpenThreadsd.lib glu32.lib opengl32.lib "
				OutputFile="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Debug\osgPlugins-2.5.0\osgdb_netd.dll"
				Version="0.0"
				LinkIncremental="2"
				AdditionalLibraryDirectories="..\..\..\lib\$(OutDir),..\..\..\lib"
				ProgramDataBaseFile="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\$(OutDir)\osgPlugins-2.5.0\osgdb_netd.pdb"
				GenerateDebugInformation="TRUE"
				ImportLibrary="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Debug\osgdb_netd.lib"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="Release"
			IntermediateDirectory="osgdb_net.dir\Release"
			ConfigurationType="2"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="FALSE"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalOptions=" /Zm1000"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				CompileAs="2"
				ExceptionHandling="1"
				InlineFunctionExpansion="2"
				Optimization="2"
				RuntimeLibrary="2"
				RuntimeTypeInfo="TRUE"
				WarningLevel="3"
				PreprocessorDefinitions="WIN32,_WINDOWS,NDEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;,osgdb_net_EXPORTS"
				AssemblerListingLocation="Release"
				ObjectFile="$(IntDir)\"
				ProgramDataBaseFileName="$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/Release/osgPlugins-2.5.0/osgdb_net.pdb"
/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCResourceCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				PreprocessorDefinitions="WIN32,_WINDOWS,NDEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;,osgdb_net_EXPORTS"/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="WIN32,_WINDOWS,NDEBUG,_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;,osgdb_net_EXPORTS"
				MkTypLibCompatible="FALSE"
				TargetEnvironment="1"
				GenerateStublessProxies="TRUE"
				TypeLibraryName="$(InputName).tlb"
				OutputDirectory="$(IntDir)"
				HeaderFileName="$(InputName).h"
				DLLDataFileName=""
				InterfaceIdentifierFileName="$(InputName)_i.c"
				ProxyFileName="$(InputName)_p.c"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCPostBuildEventTool"/>
			<Tool
				Name="VCLinkerTool"
				AdditionalOptions=" /STACK:10000000 /machine:I386"
				AdditionalDependencies="$(NOINHERIT) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OpenThreads.lib osg.lib osgDB.lib osgUtil.lib wsock32.lib osg.lib OpenThreads.lib glu32.lib opengl32.lib "
				OutputFile="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Release\osgPlugins-2.5.0\osgdb_net.dll"
				Version="0.0"
				LinkIncremental="1"
				AdditionalLibraryDirectories="..\..\..\lib\$(OutDir),..\..\..\lib"
				ProgramDataBaseFile="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\$(OutDir)\osgPlugins-2.5.0\osgdb_net.pdb"
				ImportLibrary="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\lib\Release\osgdb_net.lib"/>
		</Configuration>
	</Configurations>
	<Files>
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\CMakeLists.txt">
				<FileConfiguration
					Name="Debug|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description="Building Custom Rule $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgPlugins/net/CMakeLists.txt"
					CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -H$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph -B$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\CMakeLists.txt;&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\share\cmake-2.4\Templates\CMakeWindowsSystemConfig.cmake&quot;;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\CMakeLists.txt;"
					Outputs="osgdb_net.vcproj.cmake"/>
				</FileConfiguration>
				<FileConfiguration
					Name="Release|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description="Building Custom Rule $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgPlugins/net/CMakeLists.txt"
					CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -H$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph -B$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\CMakeLists.txt;&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\share\cmake-2.4\Templates\CMakeWindowsSystemConfig.cmake&quot;;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\CMakeLists.txt;"
					Outputs="osgdb_net.vcproj.cmake"/>
				</FileConfiguration>
			</File>
		<Filter
			Name="Source Files"
			Filter="">
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\ReaderWriterNET.cpp">
			</File>
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\sockinet.cpp">
			</File>
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\sockstream.cpp">
			</File>
		</Filter>
		<Filter
			Name="Header Files"
			Filter="">
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\sockinet.h">
			</File>
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\net\sockstream.h">
			</File>
		</Filter>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>