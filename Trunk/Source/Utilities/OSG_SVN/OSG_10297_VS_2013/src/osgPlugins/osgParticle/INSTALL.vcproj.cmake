<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="9.00"
	Name="INSTALL"
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
			IntermediateDirectory="INSTALL.dir\Debug"
			ConfigurationType="10"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="FALSE"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\include;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				ExceptionHandling="FALSE"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;"
				AssemblerListingLocation="Debug"
				ObjectFile="$(IntDir)\"
/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCResourceCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\include;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;"/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Debug\&quot;&quot;"
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
				Name="VCPostBuildEventTool"
CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -DBUILD_TYPE=$(OutDir) -P cmake_install.cmake"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="Release"
			IntermediateDirectory="INSTALL.dir\Release"
			ConfigurationType="10"
			UseOfMFC="0"
			ATLMinimizesCRunTimeLibraryUsage="FALSE"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\include;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				ExceptionHandling="FALSE"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;"
				AssemblerListingLocation="Release"
				ObjectFile="$(IntDir)\"
/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCResourceCompilerTool"
				AdditionalIncludeDirectories="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\include;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\include;"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;"/>
			<Tool
				Name="VCMIDLTool"
				PreprocessorDefinitions="_SCL_SECURE_NO_WARNINGS,_CRT_SECURE_NO_DEPRECATE,&quot;OSG_DEBUG_POSTFIX=d&quot;,&quot;CMAKE_INTDIR=\&quot;Release\&quot;&quot;"
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
				Name="VCPostBuildEventTool"
CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -DBUILD_TYPE=$(OutDir) -P cmake_install.cmake"/>
		</Configuration>
	</Configurations>
	<Files>
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\osgParticle\CMakeLists.txt">
				<FileConfiguration
					Name="Debug|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description="Building Custom Rule $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgPlugins/osgParticle/CMakeLists.txt"
					CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -H$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph -B$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\osgParticle\CMakeLists.txt;&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\share\cmake-2.4\Templates\CMakeWindowsSystemConfig.cmake&quot;;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\osgParticle\CMakeLists.txt;"
					Outputs="osgdb_osgparticle.vcproj.cmake"/>
				</FileConfiguration>
				<FileConfiguration
					Name="Release|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description="Building Custom Rule $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgPlugins/osgParticle/CMakeLists.txt"
					CommandLine="&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\bin\cmake.exe&quot; -H$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph -B$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\osgParticle\CMakeLists.txt;&quot;C:\Program Files (x86)\cmake-2.4.8-win32-x86\share\cmake-2.4\Templates\CMakeWindowsSystemConfig.cmake&quot;;$(GG_UTILITIES)\OSG_SVN\OpenSceneGraph\src\osgPlugins\osgParticle\CMakeLists.txt;"
					Outputs="osgdb_osgparticle.vcproj.cmake"/>
				</FileConfiguration>
			</File>
		<Filter
			Name="CMake Rules"
			Filter="">
			<File
				RelativePath="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\src\osgPlugins\osgParticle\CMakeFiles\INSTALL_force.rule">
				<FileConfiguration
					Name="Debug|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description=" "
					CommandLine=";"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\src\osgPlugins\osgParticle\CMakeFiles\INSTALL_force.rule"
					Outputs="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\src\osgPlugins\osgParticle\CMakeFiles\INSTALL_force"/>
				</FileConfiguration>
				<FileConfiguration
					Name="Release|Win32">
					<Tool
					Name="VCCustomBuildTool"
					Description=" "
					CommandLine=";"
					AdditionalDependencies="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\src\osgPlugins\osgParticle\CMakeFiles\INSTALL_force.rule"
					Outputs="$(GG_UTILITIES)\OSG_SVN\OSG_10297_VS_2008\src\osgPlugins\osgParticle\CMakeFiles\INSTALL_force"/>
				</FileConfiguration>
			</File>
		</Filter>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
