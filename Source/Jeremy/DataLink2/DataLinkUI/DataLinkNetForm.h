#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace DataLinkUI {

	/// <summary>
	/// Summary for DataLinkNetForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class DataLinkNetForm : public System::Windows::Forms::Form
	{
	private:
		static int fadeCounter = 0;

	public:
		DataLinkNetForm(void)
		{
			InitializeComponent();

			// Fade the GUI in from 0 to 100 opacity.
			FadeIn();

			DataEntrySwitch();
			GetNWFields();
			if (this->cboxSites->Items->Count > 0)	// Force selection to index 0 only if there are items in the drop-down.
				this->cboxSites->SelectedIndex = 0;

			MarkSaved();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DataLinkNetForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  panel1;

	private: System::Windows::Forms::Button^  btnDone;
	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::Label^  lblTitle;
	private: System::Windows::Forms::Label^  lblName;




















	private: System::Windows::Forms::Panel^  panel4;

	private: System::Windows::Forms::Panel^  panel6;
	private: System::Windows::Forms::Button^  btnDelete;
	private: System::Windows::Forms::ComboBox^  cboxSites;

	private: System::Windows::Forms::Button^  btnSave;
	private: System::Windows::Forms::Label^  lblSettings;
	private: System::Windows::Forms::Button^  btnNew;
	private: System::Windows::Forms::Panel^  panel5;
	private: System::Windows::Forms::Label^  lblEnabled;
	private: System::Windows::Forms::CheckBox^  chkEnabled;
	private: System::Windows::Forms::Label^  lblTimeout;
	private: System::Windows::Forms::Label^  lblRefresh;
	private: System::Windows::Forms::TextBox^  tboxAddress;

	private: System::Windows::Forms::TextBox^  tboxRefresh;
	private: System::Windows::Forms::TextBox^  tboxTimeout;
	private: System::Windows::Forms::TextBox^  tboxBlocksize;
	private: System::Windows::Forms::Label^  lblSite;
	private: System::Windows::Forms::Label^  lblBlocksize;
	private: System::Windows::Forms::Label^  lblSiteInfo;
	private: System::Windows::Forms::Label^  lblCopyright;
	private: System::Windows::Forms::LinkLabel^  linkNewTek;
	private: System::Windows::Forms::Button^  btnOpenSite;
	private: System::Windows::Forms::Timer^  fadeInTimer;
	private: System::Windows::Forms::Timer^  fadeOutTimer;
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::ComponentModel::IContainer^  components;















	protected: 

	protected: 

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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(DataLinkNetForm::typeid));
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->panel4 = (gcnew System::Windows::Forms::Panel());
			this->linkNewTek = (gcnew System::Windows::Forms::LinkLabel());
			this->lblCopyright = (gcnew System::Windows::Forms::Label());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->panel6 = (gcnew System::Windows::Forms::Panel());
			this->lblEnabled = (gcnew System::Windows::Forms::Label());
			this->chkEnabled = (gcnew System::Windows::Forms::CheckBox());
			this->btnDelete = (gcnew System::Windows::Forms::Button());
			this->cboxSites = (gcnew System::Windows::Forms::ComboBox());
			this->lblSettings = (gcnew System::Windows::Forms::Label());
			this->btnNew = (gcnew System::Windows::Forms::Button());
			this->panel5 = (gcnew System::Windows::Forms::Panel());
			this->btnOpenSite = (gcnew System::Windows::Forms::Button());
			this->lblTimeout = (gcnew System::Windows::Forms::Label());
			this->lblRefresh = (gcnew System::Windows::Forms::Label());
			this->tboxAddress = (gcnew System::Windows::Forms::TextBox());
			this->tboxRefresh = (gcnew System::Windows::Forms::TextBox());
			this->lblSite = (gcnew System::Windows::Forms::Label());
			this->tboxTimeout = (gcnew System::Windows::Forms::TextBox());
			this->tboxBlocksize = (gcnew System::Windows::Forms::TextBox());
			this->lblBlocksize = (gcnew System::Windows::Forms::Label());
			this->lblName = (gcnew System::Windows::Forms::Label());
			this->lblSiteInfo = (gcnew System::Windows::Forms::Label());
			this->lblTitle = (gcnew System::Windows::Forms::Label());
			this->btnSave = (gcnew System::Windows::Forms::Button());
			this->btnDone = (gcnew System::Windows::Forms::Button());
			this->fadeInTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->fadeOutTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->panel1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->panel4->SuspendLayout();
			this->panel2->SuspendLayout();
			this->panel6->SuspendLayout();
			this->panel5->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel1.BackgroundImage")));
			this->panel1->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel1->Controls->Add(this->pictureBox1);
			this->panel1->Controls->Add(this->panel4);
			this->panel1->Controls->Add(this->panel2);
			this->panel1->Controls->Add(this->btnSave);
			this->panel1->Controls->Add(this->btnDone);
			this->panel1->Location = System::Drawing::Point(0, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(541, 486);
			this->panel1->TabIndex = 0;
			// 
			// pictureBox1
			// 
			this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox1.Image")));
			this->pictureBox1->Location = System::Drawing::Point(-2, -2);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(541, 98);
			this->pictureBox1->TabIndex = 4;
			this->pictureBox1->TabStop = false;
			// 
			// panel4
			// 
			this->panel4->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel4.BackgroundImage")));
			this->panel4->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
			this->panel4->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel4->Controls->Add(this->linkNewTek);
			this->panel4->Controls->Add(this->lblCopyright);
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
			this->linkNewTek->TabIndex = 1;
			this->linkNewTek->TabStop = true;
			this->linkNewTek->Text = L"NewTek Inc.";
			this->linkNewTek->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &DataLinkNetForm::linkNewTek_LinkClicked);
			// 
			// lblCopyright
			// 
			this->lblCopyright->AutoSize = true;
			this->lblCopyright->BackColor = System::Drawing::Color::Transparent;
			this->lblCopyright->Location = System::Drawing::Point(1, 3);
			this->lblCopyright->Name = L"lblCopyright";
			this->lblCopyright->Size = System::Drawing::Size(81, 13);
			this->lblCopyright->TabIndex = 0;
			this->lblCopyright->Text = L"Copyright 2010,";
			// 
			// panel2
			// 
			this->panel2->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel2.BackgroundImage")));
			this->panel2->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel2->Controls->Add(this->panel6);
			this->panel2->Controls->Add(this->panel5);
			this->panel2->Controls->Add(this->lblName);
			this->panel2->Controls->Add(this->lblSiteInfo);
			this->panel2->Controls->Add(this->lblTitle);
			this->panel2->Location = System::Drawing::Point(4, 102);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(529, 348);
			this->panel2->TabIndex = 2;
			// 
			// panel6
			// 
			this->panel6->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel6.BackgroundImage")));
			this->panel6->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->panel6->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel6->Controls->Add(this->lblEnabled);
			this->panel6->Controls->Add(this->chkEnabled);
			this->panel6->Controls->Add(this->btnDelete);
			this->panel6->Controls->Add(this->cboxSites);
			this->panel6->Controls->Add(this->lblSettings);
			this->panel6->Controls->Add(this->btnNew);
			this->panel6->Location = System::Drawing::Point(24, 70);
			this->panel6->Name = L"panel6";
			this->panel6->Size = System::Drawing::Size(478, 109);
			this->panel6->TabIndex = 5;
			// 
			// lblEnabled
			// 
			this->lblEnabled->AutoSize = true;
			this->lblEnabled->BackColor = System::Drawing::Color::Transparent;
			this->lblEnabled->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblEnabled->Location = System::Drawing::Point(394, 10);
			this->lblEnabled->Name = L"lblEnabled";
			this->lblEnabled->Size = System::Drawing::Size(65, 21);
			this->lblEnabled->TabIndex = 1;
			this->lblEnabled->Text = L"Enabled";
			// 
			// chkEnabled
			// 
			this->chkEnabled->AutoSize = true;
			this->chkEnabled->BackColor = System::Drawing::Color::Transparent;
			this->chkEnabled->Location = System::Drawing::Point(376, 14);
			this->chkEnabled->Name = L"chkEnabled";
			this->chkEnabled->Size = System::Drawing::Size(15, 14);
			this->chkEnabled->TabIndex = 3;
			this->chkEnabled->UseVisualStyleBackColor = false;
			this->chkEnabled->CheckedChanged += gcnew System::EventHandler(this, &DataLinkNetForm::chkEnabled_CheckedChanged);
			// 
			// btnDelete
			// 
			this->btnDelete->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDelete.BackgroundImage")));
			this->btnDelete->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnDelete->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->btnDelete->Location = System::Drawing::Point(398, 70);
			this->btnDelete->Name = L"btnDelete";
			this->btnDelete->Size = System::Drawing::Size(61, 23);
			this->btnDelete->TabIndex = 1;
			this->btnDelete->Text = L"Delete";
			this->btnDelete->UseVisualStyleBackColor = true;
			this->btnDelete->Click += gcnew System::EventHandler(this, &DataLinkNetForm::btnDelete_Click);
			// 
			// cboxSites
			// 
			this->cboxSites->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->cboxSites->FormattingEnabled = true;
			this->cboxSites->ItemHeight = 15;
			this->cboxSites->Location = System::Drawing::Point(114, 8);
			this->cboxSites->Name = L"cboxSites";
			this->cboxSites->Size = System::Drawing::Size(256, 23);
			this->cboxSites->TabIndex = 2;
			this->cboxSites->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkNetForm::cboxSites_SelectedIndexChanged);
			// 
			// lblSettings
			// 
			this->lblSettings->AutoSize = true;
			this->lblSettings->BackColor = System::Drawing::Color::Transparent;
			this->lblSettings->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblSettings->Location = System::Drawing::Point(11, 8);
			this->lblSettings->Name = L"lblSettings";
			this->lblSettings->Size = System::Drawing::Size(36, 21);
			this->lblSettings->TabIndex = 1;
			this->lblSettings->Text = L"Site";
			// 
			// btnNew
			// 
			this->btnNew->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnNew.BackgroundImage")));
			this->btnNew->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnNew->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->btnNew->Location = System::Drawing::Point(398, 41);
			this->btnNew->Name = L"btnNew";
			this->btnNew->Size = System::Drawing::Size(61, 23);
			this->btnNew->TabIndex = 1;
			this->btnNew->Text = L"New";
			this->btnNew->UseVisualStyleBackColor = true;
			this->btnNew->Click += gcnew System::EventHandler(this, &DataLinkNetForm::btnNew_Click);
			// 
			// panel5
			// 
			this->panel5->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel5.BackgroundImage")));
			this->panel5->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->panel5->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel5->Controls->Add(this->btnOpenSite);
			this->panel5->Controls->Add(this->lblTimeout);
			this->panel5->Controls->Add(this->lblRefresh);
			this->panel5->Controls->Add(this->tboxAddress);
			this->panel5->Controls->Add(this->tboxRefresh);
			this->panel5->Controls->Add(this->lblSite);
			this->panel5->Controls->Add(this->tboxTimeout);
			this->panel5->Controls->Add(this->tboxBlocksize);
			this->panel5->Controls->Add(this->lblBlocksize);
			this->panel5->Location = System::Drawing::Point(24, 207);
			this->panel5->Name = L"panel5";
			this->panel5->Size = System::Drawing::Size(478, 113);
			this->panel5->TabIndex = 4;
			// 
			// btnOpenSite
			// 
			this->btnOpenSite->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnOpenSite.BackgroundImage")));
			this->btnOpenSite->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnOpenSite->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->btnOpenSite->Location = System::Drawing::Point(398, 63);
			this->btnOpenSite->Name = L"btnOpenSite";
			this->btnOpenSite->Size = System::Drawing::Size(61, 23);
			this->btnOpenSite->TabIndex = 4;
			this->btnOpenSite->Text = L"Browser";
			this->btnOpenSite->UseVisualStyleBackColor = true;
			this->btnOpenSite->Click += gcnew System::EventHandler(this, &DataLinkNetForm::btnOpenSite_Click);
			// 
			// lblTimeout
			// 
			this->lblTimeout->AutoSize = true;
			this->lblTimeout->BackColor = System::Drawing::Color::Transparent;
			this->lblTimeout->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblTimeout->Location = System::Drawing::Point(11, 63);
			this->lblTimeout->Name = L"lblTimeout";
			this->lblTimeout->Size = System::Drawing::Size(102, 21);
			this->lblTimeout->TabIndex = 1;
			this->lblTimeout->Text = L"Timeout (ms)";
			// 
			// lblRefresh
			// 
			this->lblRefresh->AutoSize = true;
			this->lblRefresh->BackColor = System::Drawing::Color::Transparent;
			this->lblRefresh->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblRefresh->Location = System::Drawing::Point(11, 36);
			this->lblRefresh->Name = L"lblRefresh";
			this->lblRefresh->Size = System::Drawing::Size(99, 21);
			this->lblRefresh->TabIndex = 1;
			this->lblRefresh->Text = L"Refresh (sec)";
			// 
			// tboxAddress
			// 
			this->tboxAddress->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tboxAddress->Location = System::Drawing::Point(114, 9);
			this->tboxAddress->Name = L"tboxAddress";
			this->tboxAddress->Size = System::Drawing::Size(345, 21);
			this->tboxAddress->TabIndex = 0;
			this->tboxAddress->WordWrap = false;
			this->tboxAddress->TextChanged += gcnew System::EventHandler(this, &DataLinkNetForm::tboxAddress_TextChanged);
			// 
			// tboxRefresh
			// 
			this->tboxRefresh->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tboxRefresh->Location = System::Drawing::Point(114, 36);
			this->tboxRefresh->Name = L"tboxRefresh";
			this->tboxRefresh->Size = System::Drawing::Size(97, 21);
			this->tboxRefresh->TabIndex = 0;
			this->tboxRefresh->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->tboxRefresh->WordWrap = false;
			this->tboxRefresh->TextChanged += gcnew System::EventHandler(this, &DataLinkNetForm::tboxRefresh_TextChanged);
			// 
			// lblSite
			// 
			this->lblSite->AutoSize = true;
			this->lblSite->BackColor = System::Drawing::Color::Transparent;
			this->lblSite->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblSite->Location = System::Drawing::Point(11, 9);
			this->lblSite->Name = L"lblSite";
			this->lblSite->Size = System::Drawing::Size(66, 21);
			this->lblSite->TabIndex = 1;
			this->lblSite->Text = L"Address";
			// 
			// tboxTimeout
			// 
			this->tboxTimeout->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tboxTimeout->Location = System::Drawing::Point(114, 63);
			this->tboxTimeout->Name = L"tboxTimeout";
			this->tboxTimeout->Size = System::Drawing::Size(97, 21);
			this->tboxTimeout->TabIndex = 0;
			this->tboxTimeout->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->tboxTimeout->WordWrap = false;
			this->tboxTimeout->TextChanged += gcnew System::EventHandler(this, &DataLinkNetForm::tboxTimeout_TextChanged);
			// 
			// tboxBlocksize
			// 
			this->tboxBlocksize->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tboxBlocksize->Location = System::Drawing::Point(362, 36);
			this->tboxBlocksize->Name = L"tboxBlocksize";
			this->tboxBlocksize->Size = System::Drawing::Size(97, 21);
			this->tboxBlocksize->TabIndex = 0;
			this->tboxBlocksize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->tboxBlocksize->WordWrap = false;
			this->tboxBlocksize->TextChanged += gcnew System::EventHandler(this, &DataLinkNetForm::tboxBlocksize_TextChanged);
			// 
			// lblBlocksize
			// 
			this->lblBlocksize->AutoSize = true;
			this->lblBlocksize->BackColor = System::Drawing::Color::Transparent;
			this->lblBlocksize->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblBlocksize->Location = System::Drawing::Point(233, 36);
			this->lblBlocksize->Name = L"lblBlocksize";
			this->lblBlocksize->Size = System::Drawing::Size(128, 21);
			this->lblBlocksize->TabIndex = 1;
			this->lblBlocksize->Text = L"Block size (bytes)";
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
			this->lblName->TabIndex = 1;
			this->lblName->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// lblSiteInfo
			// 
			this->lblSiteInfo->AutoSize = true;
			this->lblSiteInfo->BackColor = System::Drawing::Color::Transparent;
			this->lblSiteInfo->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 14.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblSiteInfo->Location = System::Drawing::Point(213, 180);
			this->lblSiteInfo->Name = L"lblSiteInfo";
			this->lblSiteInfo->Size = System::Drawing::Size(90, 25);
			this->lblSiteInfo->TabIndex = 0;
			this->lblSiteInfo->Text = L"Site Info";
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
			this->lblTitle->Text = L"Network";
			this->lblTitle->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// btnSave
			// 
			this->btnSave->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSave.BackgroundImage")));
			this->btnSave->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->btnSave->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->btnSave->Location = System::Drawing::Point(362, 455);
			this->btnSave->Name = L"btnSave";
			this->btnSave->Size = System::Drawing::Size(83, 23);
			this->btnSave->TabIndex = 1;
			this->btnSave->Text = L"Save";
			this->btnSave->UseVisualStyleBackColor = true;
			this->btnSave->Click += gcnew System::EventHandler(this, &DataLinkNetForm::btnSave_Click);
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
			this->btnDone->TabIndex = 1;
			this->btnDone->Text = L"Done";
			this->btnDone->UseVisualStyleBackColor = true;
			this->btnDone->Click += gcnew System::EventHandler(this, &DataLinkNetForm::btnDone_Click);
			// 
			// DataLinkNetForm
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
			this->Name = L"DataLinkNetForm";
			this->Opacity = 0;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"DataLink Network Configuration";
			this->panel1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->panel4->ResumeLayout(false);
			this->panel4->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->panel6->ResumeLayout(false);
			this->panel6->PerformLayout();
			this->panel5->ResumeLayout(false);
			this->panel5->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion

#pragma region GUI Elements
//	________________________________________
// O)_______________________________________)
// \				GUI Elements			\
//	\________________________________________\
//	O)________________________________________)
	System::Void btnSave_Click(System::Object^  sender, System::EventArgs^  e)
	{
		SetNWFields();
		GetNWFields();
		MarkSaved();
	}

	System::Void btnDone_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Form's cancel button.
	}

	System::Void btnNew_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->tboxAddress->Clear();
		this->cboxSites->Text = "";
		this->tboxBlocksize->Text = "4096";
		this->tboxRefresh->Text = "60";
		this->tboxTimeout->Text = "6000";
		this->chkEnabled->Checked = true;

		MarkSaved();
	}

	System::Void btnDelete_Click(System::Object^  sender, System::EventArgs^  e)
	{
		wchar_t * path = 0;
		const wchar_t * settingName = 0;
		settingName=(wchar_t*)(_alloca(sizeof(wchar_t) * 256));
		settingName=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ,  L"CurSettingName", settingName);
		string2wchar(this->cboxSites->Text);

		path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(settingName) + wcslen(wch) + 12)));
		wcscpy(path, cwsz_SettingsPath);
		wcscat(path, L"\\");
		wcscat(path, settingName);
		wcscat(path, L"\\Sites\\");

		MyReg.RegDelete(path, wch);

		GetNWFields();
		if (this->cboxSites->Items->Count > 0)
			this->cboxSites->SelectedIndex = 0;
		else
		{
			this->cboxSites->Text="";
			MarkSaved();
		}
	}

	System::Void btnOpenSite_Click(System::Object^  sender, System::EventArgs^  e)
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(this->tboxAddress->Text);
		std::wstring url(wch);
		
		// Open browser to the site in the text box so user can verify source.
		if (url.length() != 0)
			ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	System::Void chkEnabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if (!g_EnableNetworkEntries)
		{
			wchar_t * pathToUse = 0;

			pathToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_ActivePath)+(this->cboxSites->Text->Length) + 10)));
			wcscpy(pathToUse, cwsz_ActivePath);
			wcscat(pathToUse, L"\\Sites\\");
			{string2wchar(this->cboxSites->Text); wcscat(pathToUse, wch);}

			MyReg.RegWriteBool(pathToUse, L"Enabled", (this->chkEnabled->Checked));
		}
		else
			MarkUnsaved();
	}
	System::Void tboxAddress_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxRefresh_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxTimeout_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}
	System::Void tboxBlocksize_TextChanged(System::Object^  sender, System::EventArgs^  e) {MarkUnsaved();}

	System::Void cboxSites_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		wchar_t * path = 0;
		const wchar_t * siteVal = 0;
		const wchar_t * itemName = 0;
		itemName=(wchar_t*)(_alloca(sizeof(wchar_t) * 256));
		itemName=GetRegNWData(cwsz_ActivePath, L"CurSettingName", itemName);

		// Uses global var to determine if we should read the active or stored setting.
		if (g_EnableNetworkEntries)
		{
			{string2wchar(this->cboxSites->SelectedItem->ToString());
			path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(wch) + wcslen(itemName) + 12)));
			wcscpy(path, cwsz_SettingsPath); wcscat(path, L"\\"); wcscat(path, itemName); wcscat(path, L"\\Sites\\");
			wcscat(path, wch);}
		}
		else
		{
			{string2wchar(this->cboxSites->SelectedItem->ToString());
			path=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_ActivePath) + wcslen(wch) + 12)));
			wcscpy(path, cwsz_ActivePath);
			wcscat(path, L"\\Sites\\");
			wcscat(path, wch);}
		}

		{siteVal=MyReg.RegRead(path,NULL,REG_SZ,L"Address",siteVal);wchar2string(siteVal)
			this->tboxAddress->Text=ss;}
		this->tboxRefresh->Text = System::Convert::ToString((long)MyReg.RegReadNum(path, L"Refresh"));
		this->tboxTimeout->Text = System::Convert::ToString((long)MyReg.RegReadNum(path, L"Timeout"));
		this->tboxBlocksize->Text = System::Convert::ToString((long)MyReg.RegReadNum(path, L"Blocksize"));
		this->chkEnabled->Checked=(MyReg.RegReadBool(path, L"Enabled"));


		MarkSaved();
	}

	System::Void linkNewTek_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e)
	{
		// Link to the site.
		ShellExecute(NULL, L"open", L"http://www.newtek.com", NULL, NULL, SW_SHOWNORMAL);
	}
#pragma endregion

#pragma region Registry Calls
//  ________________________________________
// O)_______________________________________)
// \				Registry Calls			\
//  \________________________________________\
//  O)________________________________________)
	const wchar_t * GetRegNWData(const wchar_t * pathToUse, const wchar_t * name, const wchar_t * val)
	{
		val=MyReg.RegRead(pathToUse, NULL, REG_SZ, name, val);
		return val;
	}
	
	DWORD GetRegNWDataN(const wchar_t * pathToUse, const wchar_t * name, const wchar_t * val)
	{
 		return MyReg.RegReadNum(pathToUse, val);
	}

	// Copies a set of saved or the active network values to the registry.
	bool SetRegNWData(const wchar_t * pathToUse, const wchar_t * name, wchar_t * val)
	{
		return MyReg.RegWrite(pathToUse, REG_SZ, name, val);
	}

	// Copies a set of saved or the active network values to the registry.
	bool SetRegNWDataN(const wchar_t * pathToUse, const wchar_t * name, DWORD val)
	{
		wchar_t * Value = 0;
		Value = (wchar_t*)(_alloca(sizeof(wchar_t) * (sizeof(val) + 1)));
		_itow(val,Value,10);
		return MyReg.RegWrite(pathToUse, REG_DWORD, name, Value);
	}

#pragma endregion

#pragma region Member Functions
//	________________________________________
// O)_______________________________________)
// \			Member Functions			\
//	\________________________________________\
//	O)________________________________________)

	//
	void GetNWFields(void)
	{
		const wchar_t *val=0;
		wchar_t * pathToUse = 0;
		size_t numSiteEntries = 0;

		//
		if (g_EnableNetworkEntries)
		{
			val=GetRegNWData(cwsz_ActivePath, L"CurSettingName", val); wchar2string(val);
			this->lblTitle->Text = "Network Stored Setting";
			this->lblName->Text=ss;
			pathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_SettingsPath) + wcslen(val) + 10)));
			wcscpy(pathToUse, cwsz_SettingsPath);
			wcscat(pathToUse, L"\\");
			wcscat(pathToUse, val);
			wcscat(pathToUse, L"\\Sites");
		}
		else
		{
			val=GetRegNWData(cwsz_ActivePath, L"CurSettingName", val); wchar2string(val);
			this->lblTitle->Text = "Network Active Setting";
			this->lblName->Text = ss;
			pathToUse = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_ActivePath) + 8)));
			wcscpy(pathToUse, cwsz_ActivePath);
			wcscat(pathToUse, L"\\Sites");
		}
			
		GetRegNWSites(pathToUse);
	}

	// Save the network fields.
	void SetNWFields()
	{
		// Disable the buttons while we save.
		this->btnSave->Enabled=false;
		this->btnDone->Enabled=false;

		wchar_t * pathToUse = L"\0";

		// Make sure these are actual numbers or else use defaults.
		if (this->tboxTimeout->Text->Length != 0)
		{
			for (int i=0; i < this->tboxTimeout->Text->Length; i++)
			{
				if (!(System::Char::IsNumber(this->tboxTimeout->Text[i])))
				{
					this->tboxTimeout->Text="2000";
					break;
				}
			}
		}
		else
			this->tboxTimeout->Text="2000";

		if (this->tboxBlocksize->Text->Length != 0)
		{
			for (int i=0; i < this->tboxBlocksize->Text->Length; i++)
			{
				if (!(System::Char::IsNumber(this->tboxBlocksize->Text[i])))
				{
					this->tboxBlocksize->Text="4096";
					break;
				}
			}
		}
		else
			this->tboxBlocksize->Text="4096";

		if (this->tboxRefresh->Text->Length != 0)
		{
			for (int i=0; i < this->tboxRefresh->Text->Length; i++)
			{
				if (!(System::Char::IsNumber(this->tboxRefresh->Text[i])))
				{
					this->tboxRefresh->Text="60";
					break;
				}
			}
		}
		else
			this->tboxRefresh->Text="60";

		// Write settings to registry.
		string2wchar(this->lblName->Text);

		pathToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*((wcslen(cwsz_SettingsPath))+(wcslen(wch))+(this->cboxSites->Text->Length) + 12)));
		wcscpy(pathToUse, cwsz_SettingsPath);
		wcscat(pathToUse, L"\\");
		wcscat(pathToUse, wch);
		wcscat(pathToUse, L"\\Sites\\");

		{string2wchar(this->cboxSites->Text); wcscat(pathToUse, wch);}

		// Write it all to registry.
		{string2wchar(this->tboxAddress->Text); SetRegNWData(pathToUse, L"Address", (wchar_t *)wch);}
		MyReg.RegWriteBool(pathToUse, L"Enabled", (this->chkEnabled->Checked));
		MyReg.RegWriteNum(pathToUse, L"Blocksize", (DWORD)Convert::ToInt32(this->tboxBlocksize->Text));
		MyReg.RegWriteNum(pathToUse, L"Timeout", (DWORD)Convert::ToInt32(this->tboxTimeout->Text));
		MyReg.RegWriteNum(pathToUse, L"Refresh", (DWORD)Convert::ToInt32(this->tboxRefresh->Text));

		// Re-enable the buttons.
		this->btnSave->Enabled=true;
		this->btnDone->Enabled=true;
	}

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

	// Enables/disables GUI features based upon user context.
	void DataEntrySwitch(void)
	{
		if (!g_EnableNetworkEntries)
		{
			this->btnSave->Visible = false;
			this->btnNew->Visible = false;
			this->btnDelete->Visible = false;

			this->cboxSites->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;

			this->tboxBlocksize->Enabled = false;
			this->tboxRefresh->Enabled = false;
			this->tboxAddress->Enabled = false;
			this->tboxTimeout->Enabled = false;
		}
	}

	// Gets the network sites from registry.
	void GetRegNWSites(wchar_t * path)
	{
		// Clear the dropdown.
		this->cboxSites->Items->Clear();

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
						this->cboxSites->Items->Add(ss);
					}
				}
			}
		}
	}
#pragma endregion

	// Fade-in & fade-out: to make transitions between forms look a bit smoother.
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
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkNetForm::fadeInTimer_Tick);
	}

	void FadeOut()
	{
		fadeCounter = 0;
		fadeInTimer->Interval = 10;
		fadeInTimer->Enabled = true;
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkNetForm::fadeOutTimer_Tick);
	}
};
}
