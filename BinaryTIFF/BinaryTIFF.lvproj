<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="18008000">
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Property Name="NI.Project.Description" Type="Str"></Property>
	<Item Name="My Computer" Type="My Computer">
		<Property Name="NI.SortType" Type="Int">3</Property>
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Examples" Type="Folder">
			<Item Name="create dummy files.vi" Type="VI" URL="../Examples/create dummy files.vi"/>
			<Item Name="test class read.vi" Type="VI" URL="../Examples/test class read.vi"/>
			<Item Name="test class write C.vi" Type="VI" URL="../Examples/test class write C.vi"/>
			<Item Name="test class write DCIMG2TIFF(4ch).vi" Type="VI" URL="../Examples/test class write DCIMG2TIFF(4ch).vi"/>
			<Item Name="test class write.vi" Type="VI" URL="../Examples/test class write.vi"/>
			<Item Name="test reader.vi" Type="VI" URL="../Examples/test reader.vi"/>
			<Item Name="test class write DCIMG2TIFF rescue.vi" Type="VI" URL="../Examples/test class write DCIMG2TIFF rescue.vi"/>
			<Item Name="storage.vi" Type="VI" URL="../Examples/storage.vi"/>
			<Item Name="header inspect.vi" Type="VI" URL="../Examples/header inspect.vi"/>
		</Item>
		<Item Name="BinaryTIFF.lvlib" Type="Library" URL="../BinaryTIFF.lvlib"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="subFile Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/express/express input/FileDialogBlock.llb/subFile Dialog.vi"/>
				<Item Name="ex_CorrectErrorChain.vi" Type="VI" URL="/&lt;vilib&gt;/express/express shared/ex_CorrectErrorChain.vi"/>
			</Item>
			<Item Name="dcimg2tiff.dll" Type="Document" URL="../CLib/DCIMG2TIFF/x64/Release/dcimg2tiff.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
