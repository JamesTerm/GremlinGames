#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Diagnostics;


namespace DataLinkUI {

	Registrar MyReg;

	/// <summary>
	/// Summary for DataLinkDBForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
public ref class DataLinkDBForm : public System::Windows::Forms::Form
{
	private:
		static int fadeCounter = 0;

	public:
		DataLinkDBForm(void)
		{
			InitializeComponent();	// Initialize GUI
			
			// Fade the GUI in from 0 to 100 opacity.
			FadeIn();
			
			DataEntrySwitch();		// Switch to enable/disable the edit of an entry based upon user context.
			GetDBFields();			// Set whatever fields are blank to a common default to help-out the users.
			if (this->cboxQueryKey->Items->Count > 0)	// Force selection to index 0 only if there are items in the drop-down.
				this->cboxQueryKey->SelectedIndex = 0;
			MarkSaved();			// Save button switch to signal the user of a need to save.
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DataLinkDBForm()
		{
			if (components)
			{
				delete components;
			}
		}
private: System::Windows::Forms::Panel^  panel4;
private: System::Windows::Forms::LinkLabel^  linkNewTek;
private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::Button^  btnSave;
private: System::Windows::Forms::Button^  btnDone;
private: System::Windows::Forms::Panel^  panel2;
private: System::Windows::Forms::Label^  lblName;
private: System::Windows::Forms::Panel^  panel5;
private: System::Windows::Forms::TextBox^  tboxTimeout;
private: System::Windows::Forms::Label^  lblTimeout;
private: System::Windows::Forms::TextBox^  tboxDriver;
private: System::Windows::Forms::Label^  lblDriver;
private: System::Windows::Forms::TextBox^  tboxServer;
private: System::Windows::Forms::TextBox^  tboxDatabase;
private: System::Windows::Forms::Label^  lblDatabase;
private: System::Windows::Forms::Label^  lblPassword;
private: System::Windows::Forms::TextBox^  tboxUserID;
private: System::Windows::Forms::Label^  lblUserID;
private: System::Windows::Forms::TextBox^  tboxPassword;
private: System::Windows::Forms::Label^  lblServer;
private: System::Windows::Forms::Label^  lblQueries;
private: System::Windows::Forms::Label^  lblTitle;

private: System::Windows::Forms::Panel^  panel1;
private: System::Windows::Forms::CheckBox^  chkQueryMulti;
private: System::Windows::Forms::TextBox^  tboxQueryCommand;
private: System::Windows::Forms::ComboBox^  cboxQueryKey;
private: System::Windows::Forms::Panel^  panel3;
private: System::Windows::Forms::Label^  lblQueryCommand;
private: System::Windows::Forms::Label^  lblQueryKey;
private: System::Windows::Forms::Button^  btnQueryDelete;
private: System::Windows::Forms::Button^  btnQueryNew;
private: System::Windows::Forms::Label^  lblQueryMulti;
private: System::Windows::Forms::Button^  btnDatabaseFolder;
private: System::Windows::Forms::Timer^  fadeInTimer;
private: System::Windows::Forms::Timer^  fadeOutTimer;
private: System::Windows::Forms::PictureBox^  pictureBox1;

private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->components = (gcnew System::ComponentModel::Container());
		System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(DataLinkDBForm::typeid));
		this->panel4 = (gcnew System::Windows::Forms::Panel());
		this->linkNewTek = (gcnew System::Windows::Forms::LinkLabel());
		this->label1 = (gcnew System::Windows::Forms::Label());
		this->btnSave = (gcnew System::Windows::Forms::Button());
		this->btnDone = (gcnew System::Windows::Forms::Button());
		this->panel2 = (gcnew System::Windows::Forms::Panel());
		this->panel5 = (gcnew System::Windows::Forms::Panel());
		this->tboxTimeout = (gcnew System::Windows::Forms::TextBox());
		this->lblTimeout = (gcnew System::Windows::Forms::Label());
		this->tboxDriver = (gcnew System::Windows::Forms::TextBox());
		this->tboxServer = (gcnew System::Windows::Forms::TextBox());
		this->lblDriver = (gcnew System::Windows::Forms::Label());
		this->tboxDatabase = (gcnew System::Windows::Forms::TextBox());
		this->lblDatabase = (gcnew System::Windows::Forms::Label());
		this->lblPassword = (gcnew System::Windows::Forms::Label());
		this->tboxUserID = (gcnew System::Windows::Forms::TextBox());
		this->lblUserID = (gcnew System::Windows::Forms::Label());
		this->tboxPassword = (gcnew System::Windows::Forms::TextBox());
		this->lblServer = (gcnew System::Windows::Forms::Label());
		this->panel3 = (gcnew System::Windows::Forms::Panel());
		this->lblQueryMulti = (gcnew System::Windows::Forms::Label());
		this->btnQueryDelete = (gcnew System::Windows::Forms::Button());
		this->btnQueryNew = (gcnew System::Windows::Forms::Button());
		this->lblQueryCommand = (gcnew System::Windows::Forms::Label());
		this->lblQueryKey = (gcnew System::Windows::Forms::Label());
		this->chkQueryMulti = (gcnew System::Windows::Forms::CheckBox());
		this->cboxQueryKey = (gcnew System::Windows::Forms::ComboBox());
		this->tboxQueryCommand = (gcnew System::Windows::Forms::TextBox());
		this->lblName = (gcnew System::Windows::Forms::Label());
		this->lblQueries = (gcnew System::Windows::Forms::Label());
		this->lblTitle = (gcnew System::Windows::Forms::Label());
		this->panel1 = (gcnew System::Windows::Forms::Panel());
		this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
		this->btnDatabaseFolder = (gcnew System::Windows::Forms::Button());
		this->fadeInTimer = (gcnew System::Windows::Forms::Timer(this->components));
		this->fadeOutTimer = (gcnew System::Windows::Forms::Timer(this->components));
		this->panel4->SuspendLayout();
		this->panel2->SuspendLayout();
		this->panel5->SuspendLayout();
		this->panel3->SuspendLayout();
		this->panel1->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
		this->SuspendLayout();
		// 
		// panel4
		// 
		this->panel4->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel4.BackgroundImage")));
		this->panel4->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->panel4->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel4->Controls->Add(this->linkNewTek);
		this->panel4->Controls->Add(this->label1);
		this->panel4->Location = System::Drawing::Point(4, 455);
		this->panel4->Name = L"panel4";
		this->panel4->Size = System::Drawing::Size(155, 23);
		this->panel4->TabIndex = 3;
		// 
		// linkNewTek
		// 
		this->linkNewTek->AutoSize = true;
		this->linkNewTek->BackColor = System::Drawing::Color::Transparent;
		this->linkNewTek->Location = System::Drawing::Point(81, 3);
		this->linkNewTek->Name = L"linkNewTek";
		this->linkNewTek->Size = System::Drawing::Size(69, 13);
		this->linkNewTek->TabIndex = 0;
		this->linkNewTek->TabStop = true;
		this->linkNewTek->Text = L"NewTek Inc.";
		this->linkNewTek->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &DataLinkDBForm::linkNewTek_LinkClicked);
		// 
		// label1
		// 
		this->label1->AutoSize = true;
		this->label1->BackColor = System::Drawing::Color::Transparent;
		this->label1->Location = System::Drawing::Point(1, 3);
		this->label1->Name = L"label1";
		this->label1->Size = System::Drawing::Size(81, 13);
		this->label1->TabIndex = 2;
		this->label1->Text = L"Copyright 2010,";
		// 
		// btnSave
		// 
		this->btnSave->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSave.BackgroundImage")));
		this->btnSave->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnSave->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnSave->Location = System::Drawing::Point(274, 455);
		this->btnSave->Name = L"btnSave";
		this->btnSave->Size = System::Drawing::Size(83, 23);
		this->btnSave->TabIndex = 5;
		this->btnSave->Text = L"Save";
		this->btnSave->UseVisualStyleBackColor = true;
		this->btnSave->Click += gcnew System::EventHandler(this, &DataLinkDBForm::btnSave_Click);
		// 
		// btnDone
		// 
		this->btnDone->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDone.BackgroundImage")));
		this->btnDone->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnDone->DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->btnDone->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnDone->Location = System::Drawing::Point(450, 455);
		this->btnDone->Name = L"btnDone";
		this->btnDone->Size = System::Drawing::Size(83, 23);
		this->btnDone->TabIndex = 0;
		this->btnDone->Text = L"Done";
		this->btnDone->UseVisualStyleBackColor = true;
		this->btnDone->Click += gcnew System::EventHandler(this, &DataLinkDBForm::btnDone_Click);
		// 
		// panel2
		// 
		this->panel2->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel2.BackgroundImage")));
		this->panel2->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel2->Controls->Add(this->panel5);
		this->panel2->Controls->Add(this->panel3);
		this->panel2->Controls->Add(this->lblName);
		this->panel2->Controls->Add(this->lblQueries);
		this->panel2->Controls->Add(this->lblTitle);
		this->panel2->Location = System::Drawing::Point(4, 102);
		this->panel2->Name = L"panel2";
		this->panel2->Size = System::Drawing::Size(529, 348);
		this->panel2->TabIndex = 1;
		// 
		// panel5
		// 
		this->panel5->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel5.BackgroundImage")));
		this->panel5->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->panel5->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel5->Controls->Add(this->tboxTimeout);
		this->panel5->Controls->Add(this->lblTimeout);
		this->panel5->Controls->Add(this->tboxDriver);
		this->panel5->Controls->Add(this->tboxServer);
		this->panel5->Controls->Add(this->lblDriver);
		this->panel5->Controls->Add(this->tboxDatabase);
		this->panel5->Controls->Add(this->lblDatabase);
		this->panel5->Controls->Add(this->lblPassword);
		this->panel5->Controls->Add(this->tboxUserID);
		this->panel5->Controls->Add(this->lblUserID);
		this->panel5->Controls->Add(this->tboxPassword);
		this->panel5->Controls->Add(this->lblServer);
		this->panel5->Location = System::Drawing::Point(6, 72);
		this->panel5->Name = L"panel5";
		this->panel5->Size = System::Drawing::Size(511, 106);
		this->panel5->TabIndex = 3;
		// 
		// tboxTimeout
		// 
		this->tboxTimeout->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxTimeout->Location = System::Drawing::Point(156, 78);
		this->tboxTimeout->Name = L"tboxTimeout";
		this->tboxTimeout->Size = System::Drawing::Size(141, 21);
		this->tboxTimeout->TabIndex = 4;
		this->tboxTimeout->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxTimeout_TextChanged);
		// 
		// lblTimeout
		// 
		this->lblTimeout->AutoSize = true;
		this->lblTimeout->BackColor = System::Drawing::Color::Transparent;
		this->lblTimeout->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblTimeout->Location = System::Drawing::Point(1, 78);
		this->lblTimeout->Name = L"lblTimeout";
		this->lblTimeout->Size = System::Drawing::Size(149, 21);
		this->lblTimeout->TabIndex = 3;
		this->lblTimeout->Text = L"Query Timeout (ms)";
		// 
		// tboxDriver
		// 
		this->tboxDriver->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxDriver->Location = System::Drawing::Point(83, 53);
		this->tboxDriver->Name = L"tboxDriver";
		this->tboxDriver->Size = System::Drawing::Size(214, 21);
		this->tboxDriver->TabIndex = 1;
		this->tboxDriver->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxDriver_TextChanged);
		// 
		// tboxServer
		// 
		this->tboxServer->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxServer->Location = System::Drawing::Point(83, 28);
		this->tboxServer->Name = L"tboxServer";
		this->tboxServer->Size = System::Drawing::Size(214, 21);
		this->tboxServer->TabIndex = 1;
		this->tboxServer->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxServer_TextChanged);
		// 
		// lblDriver
		// 
		this->lblDriver->AutoSize = true;
		this->lblDriver->BackColor = System::Drawing::Color::Transparent;
		this->lblDriver->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblDriver->Location = System::Drawing::Point(1, 53);
		this->lblDriver->Name = L"lblDriver";
		this->lblDriver->Size = System::Drawing::Size(53, 21);
		this->lblDriver->TabIndex = 2;
		this->lblDriver->Text = L"Driver";
		// 
		// tboxDatabase
		// 
		this->tboxDatabase->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxDatabase->Location = System::Drawing::Point(83, 3);
		this->tboxDatabase->Name = L"tboxDatabase";
		this->tboxDatabase->Size = System::Drawing::Size(214, 21);
		this->tboxDatabase->TabIndex = 1;
		this->tboxDatabase->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxDatabase_TextChanged);
		// 
		// lblDatabase
		// 
		this->lblDatabase->AutoSize = true;
		this->lblDatabase->BackColor = System::Drawing::Color::Transparent;
		this->lblDatabase->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblDatabase->Location = System::Drawing::Point(1, 3);
		this->lblDatabase->Name = L"lblDatabase";
		this->lblDatabase->Size = System::Drawing::Size(74, 21);
		this->lblDatabase->TabIndex = 2;
		this->lblDatabase->Text = L"Database";
		// 
		// lblPassword
		// 
		this->lblPassword->AutoSize = true;
		this->lblPassword->BackColor = System::Drawing::Color::Transparent;
		this->lblPassword->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblPassword->Location = System::Drawing::Point(306, 28);
		this->lblPassword->Name = L"lblPassword";
		this->lblPassword->Size = System::Drawing::Size(77, 21);
		this->lblPassword->TabIndex = 2;
		this->lblPassword->Text = L"Password";
		// 
		// tboxUserID
		// 
		this->tboxUserID->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxUserID->Location = System::Drawing::Point(383, 3);
		this->tboxUserID->Name = L"tboxUserID";
		this->tboxUserID->Size = System::Drawing::Size(120, 21);
		this->tboxUserID->TabIndex = 1;
		this->tboxUserID->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxUserID_TextChanged);
		// 
		// lblUserID
		// 
		this->lblUserID->AutoSize = true;
		this->lblUserID->BackColor = System::Drawing::Color::Transparent;
		this->lblUserID->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblUserID->Location = System::Drawing::Point(306, 3);
		this->lblUserID->Name = L"lblUserID";
		this->lblUserID->Size = System::Drawing::Size(61, 21);
		this->lblUserID->TabIndex = 2;
		this->lblUserID->Text = L"User ID";
		// 
		// tboxPassword
		// 
		this->tboxPassword->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxPassword->Location = System::Drawing::Point(383, 28);
		this->tboxPassword->Name = L"tboxPassword";
		this->tboxPassword->PasswordChar = '*';
		this->tboxPassword->Size = System::Drawing::Size(120, 21);
		this->tboxPassword->TabIndex = 1;
		this->tboxPassword->UseSystemPasswordChar = true;
		this->tboxPassword->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxPassword_TextChanged);
		// 
		// lblServer
		// 
		this->lblServer->AutoSize = true;
		this->lblServer->BackColor = System::Drawing::Color::Transparent;
		this->lblServer->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblServer->Location = System::Drawing::Point(1, 28);
		this->lblServer->Name = L"lblServer";
		this->lblServer->Size = System::Drawing::Size(55, 21);
		this->lblServer->TabIndex = 2;
		this->lblServer->Text = L"Server";
		// 
		// panel3
		// 
		this->panel3->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel3.BackgroundImage")));
		this->panel3->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->panel3->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel3->Controls->Add(this->lblQueryMulti);
		this->panel3->Controls->Add(this->btnQueryDelete);
		this->panel3->Controls->Add(this->btnQueryNew);
		this->panel3->Controls->Add(this->lblQueryCommand);
		this->panel3->Controls->Add(this->lblQueryKey);
		this->panel3->Controls->Add(this->chkQueryMulti);
		this->panel3->Controls->Add(this->cboxQueryKey);
		this->panel3->Controls->Add(this->tboxQueryCommand);
		this->panel3->Location = System::Drawing::Point(6, 202);
		this->panel3->Name = L"panel3";
		this->panel3->Size = System::Drawing::Size(511, 139);
		this->panel3->TabIndex = 10;
		// 
		// lblQueryMulti
		// 
		this->lblQueryMulti->AutoSize = true;
		this->lblQueryMulti->BackColor = System::Drawing::Color::Transparent;
		this->lblQueryMulti->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblQueryMulti->Location = System::Drawing::Point(392, 6);
		this->lblQueryMulti->Name = L"lblQueryMulti";
		this->lblQueryMulti->Size = System::Drawing::Size(91, 19);
		this->lblQueryMulti->TabIndex = 14;
		this->lblQueryMulti->Text = L"Multiple Keys";
		// 
		// btnQueryDelete
		// 
		this->btnQueryDelete->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnQueryDelete.BackgroundImage")));
		this->btnQueryDelete->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnQueryDelete->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnQueryDelete->Location = System::Drawing::Point(155, 4);
		this->btnQueryDelete->Name = L"btnQueryDelete";
		this->btnQueryDelete->Size = System::Drawing::Size(61, 23);
		this->btnQueryDelete->TabIndex = 13;
		this->btnQueryDelete->Text = L"Delete";
		this->btnQueryDelete->UseVisualStyleBackColor = true;
		this->btnQueryDelete->Click += gcnew System::EventHandler(this, &DataLinkDBForm::btnQueryDelete_Click);
		// 
		// btnQueryNew
		// 
		this->btnQueryNew->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnQueryNew.BackgroundImage")));
		this->btnQueryNew->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnQueryNew->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnQueryNew->Location = System::Drawing::Point(89, 4);
		this->btnQueryNew->Name = L"btnQueryNew";
		this->btnQueryNew->Size = System::Drawing::Size(61, 23);
		this->btnQueryNew->TabIndex = 12;
		this->btnQueryNew->Text = L"New";
		this->btnQueryNew->UseVisualStyleBackColor = true;
		this->btnQueryNew->Click += gcnew System::EventHandler(this, &DataLinkDBForm::btnQueryNew_Click);
		// 
		// lblQueryCommand
		// 
		this->lblQueryCommand->AutoSize = true;
		this->lblQueryCommand->BackColor = System::Drawing::Color::Transparent;
		this->lblQueryCommand->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblQueryCommand->Location = System::Drawing::Point(218, 6);
		this->lblQueryCommand->Name = L"lblQueryCommand";
		this->lblQueryCommand->Size = System::Drawing::Size(73, 19);
		this->lblQueryCommand->TabIndex = 11;
		this->lblQueryCommand->Text = L"Command";
		// 
		// lblQueryKey
		// 
		this->lblQueryKey->AutoSize = true;
		this->lblQueryKey->BackColor = System::Drawing::Color::Transparent;
		this->lblQueryKey->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblQueryKey->Location = System::Drawing::Point(0, 6);
		this->lblQueryKey->Name = L"lblQueryKey";
		this->lblQueryKey->Size = System::Drawing::Size(31, 19);
		this->lblQueryKey->TabIndex = 10;
		this->lblQueryKey->Text = L"Key";
		// 
		// chkQueryMulti
		// 
		this->chkQueryMulti->AutoSize = true;
		this->chkQueryMulti->BackColor = System::Drawing::Color::Transparent;
		this->chkQueryMulti->Location = System::Drawing::Point(488, 9);
		this->chkQueryMulti->Name = L"chkQueryMulti";
		this->chkQueryMulti->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
		this->chkQueryMulti->Size = System::Drawing::Size(15, 14);
		this->chkQueryMulti->TabIndex = 9;
		this->chkQueryMulti->UseVisualStyleBackColor = false;
		this->chkQueryMulti->CheckedChanged += gcnew System::EventHandler(this, &DataLinkDBForm::chkQueryMulti_CheckedChanged);
		// 
		// cboxQueryKey
		// 
		this->cboxQueryKey->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cboxQueryKey->FormattingEnabled = true;
		this->cboxQueryKey->Location = System::Drawing::Point(4, 29);
		this->cboxQueryKey->Name = L"cboxQueryKey";
		this->cboxQueryKey->Size = System::Drawing::Size(212, 23);
		this->cboxQueryKey->TabIndex = 7;
		this->cboxQueryKey->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkDBForm::cboxQueryKey_SelectedIndexChanged);
		this->cboxQueryKey->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::cboxQueryKey_TextChanged);
		// 
		// tboxQueryCommand
		// 
		this->tboxQueryCommand->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxQueryCommand->Location = System::Drawing::Point(222, 29);
		this->tboxQueryCommand->Multiline = true;
		this->tboxQueryCommand->Name = L"tboxQueryCommand";
		this->tboxQueryCommand->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
		this->tboxQueryCommand->Size = System::Drawing::Size(281, 101);
		this->tboxQueryCommand->TabIndex = 8;
		this->tboxQueryCommand->TextChanged += gcnew System::EventHandler(this, &DataLinkDBForm::tboxQueryCommand_TextChanged);
		// 
		// lblName
		// 
		this->lblName->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
			| System::Windows::Forms::AnchorStyles::Left) 
			| System::Windows::Forms::AnchorStyles::Right));
		this->lblName->BackColor = System::Drawing::Color::Transparent;
		this->lblName->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblName->Location = System::Drawing::Point(8, 29);
		this->lblName->Name = L"lblName";
		this->lblName->Size = System::Drawing::Size(509, 43);
		this->lblName->TabIndex = 4;
		this->lblName->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// lblQueries
		// 
		this->lblQueries->AutoSize = true;
		this->lblQueries->BackColor = System::Drawing::Color::Transparent;
		this->lblQueries->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 14.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblQueries->Location = System::Drawing::Point(219, 176);
		this->lblQueries->Name = L"lblQueries";
		this->lblQueries->Size = System::Drawing::Size(84, 25);
		this->lblQueries->TabIndex = 0;
		this->lblQueries->Text = L"Queries";
		// 
		// lblTitle
		// 
		this->lblTitle->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
			| System::Windows::Forms::AnchorStyles::Left) 
			| System::Windows::Forms::AnchorStyles::Right));
		this->lblTitle->BackColor = System::Drawing::Color::Transparent;
		this->lblTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 14.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblTitle->Location = System::Drawing::Point(6, 3);
		this->lblTitle->Name = L"lblTitle";
		this->lblTitle->Size = System::Drawing::Size(511, 25);
		this->lblTitle->TabIndex = 0;
		this->lblTitle->Text = L"Database";
		this->lblTitle->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// panel1
		// 
		this->panel1->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel1.BackgroundImage")));
		this->panel1->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel1->Controls->Add(this->pictureBox1);
		this->panel1->Controls->Add(this->panel2);
		this->panel1->Controls->Add(this->btnDone);
		this->panel1->Controls->Add(this->btnDatabaseFolder);
		this->panel1->Controls->Add(this->btnSave);
		this->panel1->Controls->Add(this->panel4);
		this->panel1->Location = System::Drawing::Point(0, 0);
		this->panel1->Name = L"panel1";
		this->panel1->Size = System::Drawing::Size(541, 486);
		this->panel1->TabIndex = 2;
		// 
		// pictureBox1
		// 
		this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox1.Image")));
		this->pictureBox1->Location = System::Drawing::Point(-2, -2);
		this->pictureBox1->Name = L"pictureBox1";
		this->pictureBox1->Size = System::Drawing::Size(541, 98);
		this->pictureBox1->TabIndex = 6;
		this->pictureBox1->TabStop = false;
		// 
		// btnDatabaseFolder
		// 
		this->btnDatabaseFolder->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDatabaseFolder.BackgroundImage")));
		this->btnDatabaseFolder->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnDatabaseFolder->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnDatabaseFolder->Location = System::Drawing::Point(362, 455);
		this->btnDatabaseFolder->Name = L"btnDatabaseFolder";
		this->btnDatabaseFolder->Size = System::Drawing::Size(83, 23);
		this->btnDatabaseFolder->TabIndex = 5;
		this->btnDatabaseFolder->Text = L"DB Folder";
		this->btnDatabaseFolder->UseVisualStyleBackColor = true;
		this->btnDatabaseFolder->Click += gcnew System::EventHandler(this, &DataLinkDBForm::btnDatabaseFolder_Click);
		// 
		// DataLinkDBForm
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->BackColor = System::Drawing::Color::Silver;
		this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->ClientSize = System::Drawing::Size(541, 488);
		this->Controls->Add(this->panel1);
		this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
		this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
		this->MaximizeBox = false;
		this->MaximumSize = System::Drawing::Size(547, 512);
		this->MinimizeBox = false;
		this->MinimumSize = System::Drawing::Size(547, 512);
		this->Name = L"DataLinkDBForm";
		this->Opacity = 0;
		this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
		this->Text = L"DataLink Database Configuration";
		this->panel4->ResumeLayout(false);
		this->panel4->PerformLayout();
		this->panel2->ResumeLayout(false);
		this->panel2->PerformLayout();
		this->panel5->ResumeLayout(false);
		this->panel5->PerformLayout();
		this->panel3->ResumeLayout(false);
		this->panel3->PerformLayout();
		this->panel1->ResumeLayout(false);
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
		this->ResumeLayout(false);

	}
#pragma endregion

  /////////////////////////////////////////////////////////////////////
 //							GUI Elements							//
/////////////////////////////////////////////////////////////////////

	// Load key, command, and multi flag upon user changing drop-down selection.
	System::Void cboxQueryKey_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		wchar_t * path = 0;
		const wchar_t * commandVal = 0;
		const wchar_t * itemName = 0;
		itemName=(wchar_t*)(_alloca(sizeof(wchar_t) * 256));
		itemName=GetRegDBData(cwsz_ActivePath, L"CurSettingName", itemName);

		// Uses global var to determine if we should read the active or stored setting.
		if (g_EnableDatabaseEntries)
		{
			{string2wchar(this->cboxQueryKey->SelectedItem->ToString());
			path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(wch) + wcslen(itemName) + 12)));
			wcscpy(path, cwsz_SettingsPath); wcscat(path, L"\\"); wcscat(path, itemName); wcscat(path, L"\\Query\\");
			wcscat(path, wch);}
		}
		else
		{
			{string2wchar(this->cboxQueryKey->SelectedItem->ToString());
			path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_ActivePath) + wcslen(wch) + 12)));
			wcscpy(path, cwsz_ActivePath);
			wcscat(path, L"\\Query\\");
			wcscat(path, wch);}
		}

		{commandVal=MyReg.RegRead(path,NULL,REG_SZ,L"Command",commandVal);wchar2string(commandVal)
			this->tboxQueryCommand->Text=ss;}

		this->chkQueryMulti->Checked=(MyReg.RegReadBool(path, L"Multi"));
		MarkSaved();
	}

	// Button: Save database settings.
	System::Void btnSave_Click(System::Object^  sender, System::EventArgs^  e)
	{
		SetDBFields();
		GetDBFields();
		MarkSaved();
	}


	// Button: [DEBUG] Opens the database input folder in LiveText install directory.
	System::Void btnDatabaseFolder_Click(System::Object^  sender, System::EventArgs^  e)
	{
#ifdef _DEBUG
		// Button: [DEBUG] Opens the database input folder in LiveText install directory.
		char exe_path[MAX_PATH];

		if (GetModuleFileNameA(NULL, exe_path, MAX_PATH))
		{
			char *lastSlash = strrchr(exe_path, '\\');
			if (lastSlash++)
				*lastSlash = 0;
			strcat(exe_path, "Database Input");
			System::String ^path_string = gcnew System::String(exe_path);
			Process::Start("explorer.exe", path_string);
			delete path_string;
		}
		else
			assert(false);
#else
		const wchar_t * tempVal = 0;

		tempVal = MyReg.RegRead2(cwsz_LiveTextPath, NULL, REG_SZ, TEXT("Directory"), tempVal);

		// If LiveText is installed and found, start it up.
		if (wcslen(tempVal) > 0)
		{
			const wchar_t * tempFolder = L"\\Stats Plugins\\Database Input";
			wchar_t * tempPath = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(tempVal) + wcslen(tempFolder) + 1)));
			wcscpy(tempPath, tempVal);
			wcscat(tempPath, tempFolder);
			wchar2string(tempPath);

			Process::Start("explorer.exe", ss);
		}
		else
			assert(false);
#endif
	}

	// Button: New query
	System::Void btnQueryNew_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->cboxQueryKey->Text="";
		this->tboxQueryCommand->Clear();
		this->chkQueryMulti->Checked=false;
		MarkSaved();
	}

	// Button: Deletes currently selected key/query and updates the dropdown list.
	System::Void btnQueryDelete_Click(System::Object^  sender, System::EventArgs^  e)
	{
		wchar_t * path = 0;
		const wchar_t * settingName = 0;
		settingName=(wchar_t*)(_alloca(sizeof(wchar_t) * 256));
		settingName=GetRegDBData(cwsz_ActivePath, L"CurSettingName", settingName);
		string2wchar(this->cboxQueryKey->Text);

		path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(settingName) + wcslen(wch) + 12)));
		wcscpy(path, cwsz_SettingsPath);
		wcscat(path, L"\\");
		wcscat(path, settingName);
		wcscat(path, L"\\Query");

		MyReg.RegDelete(path, wch);

		GetDBFields();
		if (this->cboxQueryKey->Items->Count > 0)
			this->cboxQueryKey->SelectedIndex = 0;
		else
		{
			this->cboxQueryKey->Text="";
			this->tboxQueryCommand->Clear();
			MarkSaved();
		}
	}

	// Fields that, when changed, mark the save button to indicate the need to save.
	System::Void tboxDriver_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxServer_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxDatabase_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxUserID_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxPassword_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxQuery_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxTimeout_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxQueryCommand_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void chkQueryMulti_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	
	System::Void cboxQueryKey_TextChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->cboxQueryKey->Text == "")
			MarkSaved();
		else
			MarkUnsaved();
	}

	// NewTek http link.
	System::Void linkNewTek_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e)
	{
		// Link to the site.
		ShellExecute(NULL, L"open", L"http://www.newtek.com", NULL, NULL, SW_SHOWNORMAL);
	}

  /////////////////////////////////////////////////////////////////////
 //							GUI Element Code						//
/////////////////////////////////////////////////////////////////////
	
	// Reads database info and query fields.
	void GetDBFields(void)
	{
		const wchar_t *val=0;
		wchar_t * pathToUse = 0;
		size_t numQueryEntries = 0;
		
		// If we can edit the database entries, we must be loading a setting and not the currently active values...
		if (g_EnableDatabaseEntries)
		{
			val=GetRegDBData(cwsz_ActivePath, L"CurSettingName", val); wchar2string(val);
			this->lblTitle->Text = "Database Stored Setting";
			this->lblName->Text=ss;
			pathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(val) + 10)));
			wcscpy(pathToUse, cwsz_SettingsPath);
			wcscat(pathToUse, L"\\");
			wcscat(pathToUse, val);

			{val=GetRegDBData(pathToUse, L"Driver", val); wchar2string(val); this->tboxDriver->Text=ss;}
			{val=GetRegDBData(pathToUse, L"Server", val); wchar2string(val); this->tboxServer->Text=ss;}
			{val=GetRegDBData(pathToUse, L"Database", val); wchar2string(val); this->tboxDatabase->Text=ss;}
			{val=GetRegDBData(pathToUse, L"UserID", val); wchar2string(val); this->tboxUserID->Text=ss;}
			{val=GetRegDBData(pathToUse, L"Password", val); wchar2string(val); this->tboxPassword->Text=ss;}
			{val=GetRegDBData(pathToUse, L"Timeout", val); wchar2string(val); this->tboxTimeout->Text=ss;}

			wcscat(pathToUse, L"\\Query");
		}	
		// ...else we are loading the currently active values in a read-only state.
		else
		{
			val=GetRegDBData(cwsz_ActivePath, L"CurSettingName", val); wchar2string(val);
			this->lblTitle->Text = "Database Active Setting";
			this->lblName->Text = ss;
			pathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_ActivePath) + 8)));
			wcscpy(pathToUse, cwsz_ActivePath);
			wcscat(pathToUse, L"\\Query");
			
			{val=GetRegDBData(cwsz_ActivePath, L"Driver", val); wchar2string(val); this->tboxDriver->Text=ss;}
			{val=GetRegDBData(cwsz_ActivePath, L"Server", val); wchar2string(val); this->tboxServer->Text=ss;}
			{val=GetRegDBData(cwsz_ActivePath, L"Database", val); wchar2string(val); this->tboxDatabase->Text=ss;}
			{val=GetRegDBData(cwsz_ActivePath, L"User", val); wchar2string(val); this->tboxUserID->Text=ss;}
			{val=GetRegDBData(cwsz_ActivePath, L"Password", val); wchar2string(val); this->tboxPassword->Text=ss;}
			{val=GetRegDBData(cwsz_ActivePath, L"Timeout", val); wchar2string(val); this->tboxTimeout->Text=ss;}
		}
		
		// Get queries.
		GetRegDBQueries(pathToUse);
		
		// If entries are blank, populate with some common values.
		if (this->tboxDriver->Text=="") {this->tboxDriver->Text="{MySQL ODBC 5.1 Driver}";}
		if (this->tboxServer->Text=="") {this->tboxServer->Text="localhost";}
		if (this->tboxUserID->Text=="") {this->tboxUserID->Text="root";}
		if (this->tboxPassword->Text=="") {this->tboxPassword->Text="password";}
		if (this->tboxTimeout->Text=="") {this->tboxTimeout->Text="0";}
	}

	// Write the database info and query fields.
	void SetDBFields()
	{
		// Disable save and done buttons while we write entries to registry.
		this->btnSave->Enabled=false;
		this->btnDone->Enabled=false;
		
		// Sets the active fields
		wchar_t * pathToUse = 0;
		wchar_t * queryPathToUse = 0;
		wchar_t * val = 0;
		size_t arrayNumQueries = this->tboxQueryCommand->Lines->Length;

		// Forces the user to use only numbers for timeout value.
		for (int i=0; i < this->tboxTimeout->Text->Length; i++)
		{
			if (!(System::Char::IsNumber(this->tboxTimeout->Text[i])))
			{
				this->tboxTimeout->Text="0";
				break;
			}
		}

		// Keep timeout values under 100,000 ms and 0 if empty.
		if (this->tboxTimeout->Text->Length >= 6 || this->tboxTimeout->Text->Length == 0)
			this->tboxTimeout->Text="0";

		// Write database info to settings entry in registry.
		string2wchar(this->lblName->Text);
		pathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(wch) + 3)));
		wcscpy(pathToUse, cwsz_SettingsPath);
		wcscat(pathToUse, L"\\");
		wcscat(pathToUse, wch);

		{string2wchar(this->tboxDriver->Text); SetRegDBData(pathToUse, L"Driver", (wchar_t *)wch);}
		{string2wchar(this->tboxServer->Text); SetRegDBData(pathToUse, L"Server", (wchar_t *)wch);}
		{string2wchar(this->tboxDatabase->Text); SetRegDBData(pathToUse, L"Database", (wchar_t *)wch);}
		{string2wchar(this->tboxUserID->Text); SetRegDBData(pathToUse, L"UserID", (wchar_t *)wch);}
		{string2wchar(this->tboxPassword->Text); SetRegDBData(pathToUse, L"Password", (wchar_t *)wch);}
		{string2wchar(this->tboxTimeout->Text); SetRegDBData(pathToUse, L"Timeout", (wchar_t *)wch);}

		// Keep the number of lines of text in the query text box to under 1,000.
		if (arrayNumQueries > 0 && arrayNumQueries <= 999)
		{
			// Create a single string from a possibly multi-line SQL command sentence.
			queryPathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(pathToUse) + 300)));
			size_t bufSize = (wcslen(pathToUse) + 16);
			int num=0;
			num =  swprintf(queryPathToUse, bufSize, L"%ls", pathToUse);
			num += swprintf(queryPathToUse + num, bufSize - num, L"%ls", L"\\Query");
			wcscat(queryPathToUse, L"\\");

			{string2wchar(this->cboxQueryKey->Text); wcscat(queryPathToUse, wch);}
			{string2wchar(this->cboxQueryKey->Text); MyReg.RegDelete(queryPathToUse, wch);}

			MyReg.RegWriteBool(queryPathToUse, L"Multi", (this->chkQueryMulti->Checked));

			// Create an array of query lines.
			// Insert spaces where needed and <CR>/<LF> and concat into one long command string.
			array<String^>^ arrayQuery = gcnew array<String^>(arrayNumQueries);
			arrayQuery = this->tboxQueryCommand->Lines;
			System::String ^ strQuery = arrayQuery[0];
			for (size_t i=1; i<arrayNumQueries; i++)
			{
				size_t len = (strQuery->Length) - 1;
				if (strQuery[len] == ' ')
					strQuery = String::Concat(strQuery, "\r\n", arrayQuery[i]);
				else
					strQuery = String::Concat(strQuery, " \r\n", arrayQuery[i]);
			}

			// Write one long command string which contains line feeds and carriage returns.
			SetRegDBQueries(queryPathToUse, "Command", strQuery);
		}
		else if (arrayNumQueries > 999)
		{
			assert(false);
			// Need some error handling here - limit entries to 999 for now.
		}
		
		// Enable save and done buttons once we have finished saving.
		this->btnSave->Enabled=true;
		this->btnDone->Enabled=true;
	}


  /////////////////////////////////////////////////////////////////////
 //							Registry Calls							//
/////////////////////////////////////////////////////////////////////
	
	// Gets the database saved or active connect values from registry.
	const wchar_t * GetRegDBData(const wchar_t * pathToUse, const wchar_t * name, const wchar_t * val)
	{
		val=MyReg.RegRead(pathToUse, NULL, REG_SZ, name, val);
		return val;
	}

	// Copies a set of saved or the active database connect values to the registry.
	bool SetRegDBData(const wchar_t * pathToUse, const wchar_t * name, wchar_t * val)
	{
		return MyReg.RegWrite(pathToUse, REG_SZ, name, val);
	}
	
	// Copies a set of saved or the active database connect values to the registry.
	bool SetRegDBDataN(const wchar_t * pathToUse, const wchar_t * name, DWORD val)
	{
		wchar_t * Value = 0;
		Value = (wchar_t*)(_alloca(sizeof(wchar_t) * (sizeof(val) + 1)));
		_itow(val,Value,10);
		return MyReg.RegWrite(pathToUse, REG_SZ, name, Value);
	}

	// Gets the database queries from registry.
	void GetRegDBQueries(wchar_t * path)
	{
		// Clear the dropdown.
		this->cboxQueryKey->Items->Clear();

		// Create dropdown list for names of saved settings.
		const wchar_t * val=0;

		// Returns the number of settings entries in the registry.
		HKEY hKey;
		DWORD retCode;

		retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
								path,
								0,
								KEY_READ,
								&hKey);
		if (retCode == ERROR_SUCCESS)
		{
			TCHAR		achKey[MAX_KEY_LENGTH];			// buffer for subkey name
			DWORD		cbName;							// size of name string
			TCHAR		achClass[MAX_PATH] = TEXT("");	// buffer for class name
			DWORD		cchClassName = MAX_PATH;		// size of class string
			DWORD		cSubKeys=0;						// number of subkeys
			DWORD		cbMaxSubKey;					// longest subkey size
			DWORD		cchMaxClass;					// longest class string
			DWORD		cValues;						// number of values for key
			DWORD		cchMaxValue;					// longest value name
			DWORD		cbMaxValueData;					// longest value data
			DWORD		cbSecurityDescriptor;			// size of security descriptor
			FILETIME	ftLastWriteTime;				// last write time

			// 			TCHAR achValue[MAX_VALUE_NAME];
			DWORD cchValue = MAX_VALUE_NAME;

			// Get the class name and the value count. 
			retCode = RegQueryInfoKey(
				hKey,                    // key handle
				achClass,                // buffer for class name
				&cchClassName,           // size of class string
				NULL,                    // reserved
				&cSubKeys,               // number of subkeys
				&cbMaxSubKey,            // longest subkey size
				&cchMaxClass,            // longest class string
				&cValues,                // number of values for this key
				&cchMaxValue,            // longest value name
				&cbMaxValueData,         // longest value data
				&cbSecurityDescriptor,   // security descriptor
				&ftLastWriteTime);       // last write time

			if (cSubKeys)
			{
				for (size_t i=0; i<cSubKeys; i++)
				{ 
					cbName = MAX_KEY_LENGTH;
					retCode = RegEnumKeyEx(	hKey, i,
											achKey,
											&cbName,
											NULL,
											NULL,
											NULL,
											&ftLastWriteTime);
					if (retCode == ERROR_SUCCESS)
					{
						wchar2string(achKey);
						this->cboxQueryKey->Items->Add(ss);
					}
				}
			}
		}
	}

	// Sets passed string values to passed path in the registry.
	void SetRegDBQueries(const wchar_t * pathToUse, System::String ^ key, System::String ^ val)
	{
		wchar_t * Key = 0;
		wchar_t * Val = 0;

		size_t bufKeySize = key->Length;
		size_t bufValSize = val->Length;

		Val = (wchar_t*)(_alloca(sizeof(wchar_t) * (bufValSize + 1)));
		{string2wchar(val); wcscpy(Val,wch);}
		Key = (wchar_t*)(_alloca(sizeof(wchar_t) * (bufKeySize + 1)));
		{string2wchar(key); wcscpy(Key,wch);}

		MyReg.RegWrite(pathToUse, REG_SZ, Key, Val);
	}

  /////////////////////////////////////////////////////////////////////
 //								Switches							//
/////////////////////////////////////////////////////////////////////
	
	// Mark a save button indicating a change requires saving.
	void MarkUnsaved(void)
	{
		this->btnSave->Text="Save*";
		this->btnSave->Font=(gcnew System::Drawing::Font(	L"Microsoft Sans Serif",
															8.25F,
															System::Drawing::FontStyle::Bold,
															System::Drawing::GraphicsUnit::Point, 
															static_cast<System::Byte>(0)));
	}

	// Mark a save button as saved or unchanged - not needing to save.
	void MarkSaved(void)
	{
		this->btnSave->Text="Save";
		this->btnSave->Font=(gcnew System::Drawing::Font(	L"Microsoft Sans Serif",
															8.25F,
															System::Drawing::FontStyle::Regular,
															System::Drawing::GraphicsUnit::Point, 
															static_cast<System::Byte>(0)));
	}

	// Enables and/or makes visible GUI elements based upon user context - forces editing stored settings only and not the currently active values.
	void DataEntrySwitch(void)
	{
		if (!g_EnableDatabaseEntries)
		{
			this->btnSave->Visible=g_EnableDatabaseEntries;
			this->btnQueryDelete->Visible=g_EnableDatabaseEntries;
			this->btnQueryNew->Visible=g_EnableDatabaseEntries;

 			this->tboxDatabase->Enabled=g_EnableDatabaseEntries;
			this->tboxDriver->Enabled=g_EnableDatabaseEntries;
			this->tboxPassword->Enabled=g_EnableDatabaseEntries;
			this->tboxServer->Enabled=g_EnableDatabaseEntries;
			this->tboxUserID->Enabled=g_EnableDatabaseEntries;
			this->tboxTimeout->Enabled=g_EnableDatabaseEntries;

			this->cboxQueryKey->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->tboxQueryCommand->ReadOnly=true;
			this->chkQueryMulti->Enabled=g_EnableDatabaseEntries;
		}
	}

private:
	System::Void fadeInTimer_Tick(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->Opacity < 1.0f)
			this->Opacity += 0.1f;
		else
		{
			this->Opacity = 1.0f;
			fadeInTimer->Enabled=false;
		}
	}

private:
	System::Void fadeOutTimer_Tick(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->Opacity > 0.0f)
			this->Opacity = (this->Opacity - .005f) * 0.075f;
		else
		{
			this->Opacity = 0.0f;
			fadeInTimer->Enabled=false;
		}
	}

	void FadeIn()
	{
		fadeCounter = 0;
		fadeInTimer->Interval = 10;
		fadeInTimer->Enabled = true;
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkDBForm::fadeInTimer_Tick);
	}

	void FadeOut()
	{
		fadeCounter = 0;
		fadeInTimer->Interval = 10;
		fadeInTimer->Enabled = true;
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkDBForm::fadeOutTimer_Tick);
	}


private:
	System::Void btnDone_Click(System::Object^  sender, System::EventArgs^  e)
	{
		FadeOut();
	}
};

}
