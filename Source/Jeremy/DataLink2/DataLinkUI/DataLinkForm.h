#pragma once
#include "DataLinkUI.h"
#include "DataLinkDBForm.h"
#include "DataLinkNetForm.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::Timers;

namespace DataLinkUI
{

/// <summary>
/// Summary for DataLinkForm
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
/// </summary>
public ref class DataLinkForm : public System::Windows::Forms::Form
{
	private:
		static int fadeCounter = 0;

	public:
		DataLinkForm(void)
		{
			InitializeComponent();

			// Fade the GUI in from 0 to 100 opacity.
			FadeIn();

			PopulateActiveForLoad();
			PopulateSettingsNames();
#ifdef _DEBUG
			// If in debug mode, enable the "use text input" check box in the GUI for testing.
			this->chkUseText->Visible = true;
#endif
			// Disable input from text switch just in case it was somehow left set (crash etc.).
			MyReg.RegWriteBool(cwsz_ActivePath,cwsz_UseTextFile,0);

			// Populate drop-down lists for supported boards and connection types.
			array<System::String^>^ SupportedBoards =
			{
				"Generic",
				"Daktronics",
				"DSI",
				"OES",
				"TransLux",
				"WhiteWay"
			};
			
			// Populate drop-down list.
			this->cmbSettingsBoard->Items->AddRange(SupportedBoards);
			
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DataLinkForm()
		{
			if (components)
			{
				delete components;
			}
		}		
	private:
		System::Windows::Forms::Button^  btnActivate;

		System::Windows::Forms::Button^  btnClearActive;
		System::Windows::Forms::Button^  btnDelete;
		System::Windows::Forms::Button^  btnExit;
		System::Windows::Forms::Button^  btnNew;
		System::Windows::Forms::Button^  btnSave;
		System::Windows::Forms::ComboBox^  cmbSettingsBoard;
		System::Windows::Forms::ComboBox^  cmbSettingsName;
		System::Windows::Forms::ComboBox^  cmbSettingsSport;
		System::Windows::Forms::ComboBox^  cmbSettingsSportCode;
		System::Windows::Forms::Label^  lblCopyright;









		System::Windows::Forms::Label^  lblSettingsBoard;
		System::Windows::Forms::Label^  lblSettingsName;
		System::Windows::Forms::Label^  lblSettingsSport;
		System::Windows::Forms::Label^  lblSettingsTitle;
		System::Windows::Forms::Label^  lblSettingsType;









		System::Windows::Forms::LinkLabel^  linkNewTek;
		System::Windows::Forms::Panel^  panel1;
		System::Windows::Forms::Panel^  panel2;

		System::Windows::Forms::Panel^  panel4;





		System::Windows::Forms::Label^  lblSettingsParity;
		System::Windows::Forms::Label^  lblSettingsDataBits;
		System::Windows::Forms::Label^  lblSettingsPort;
		System::Windows::Forms::Label^  lblSettingsStopBits;
		System::Windows::Forms::Label^  lblSettingsBaud;
		System::Windows::Forms::ComboBox^  cmbSettingsBaud;
		System::Windows::Forms::ComboBox^  cmbSettingsParity;
		System::Windows::Forms::ComboBox^  cmbSettingsPort;
		System::Windows::Forms::ComboBox^  cmbSettingsStopBits;
		System::Windows::Forms::ComboBox^  cmbSettingsDataBits;
		System::Windows::Forms::Button^  btnSettingsDatabase;
		System::Windows::Forms::CheckBox^  chkSettingsDatabase;
		System::Windows::Forms::CheckBox^  chkSettingsNetwork;
		System::Windows::Forms::CheckBox^  chkSettingsSerial;




		System::Windows::Forms::Button^  btnSettingsNetwork;
		System::Windows::Forms::Label^  lblSettingsSerial;

private: System::Windows::Forms::Timer^  fadeInTimer;
private: System::Windows::Forms::Timer^  fadeOutTimer;

private: System::Windows::Forms::Panel^  panel3;
private: System::Windows::Forms::CheckBox^  chkUseText;
private: System::Windows::Forms::CheckBox^  chkActiveDatabase;
private: System::Windows::Forms::Button^  btnActiveLiveText;
private: System::Windows::Forms::Label^  tboxActiveDataBits;
private: System::Windows::Forms::CheckBox^  chkActiveNetwork;
private: System::Windows::Forms::Label^  tboxActiveStopBits;
private: System::Windows::Forms::CheckBox^  chkActiveSerial;
private: System::Windows::Forms::Button^  btnActiveText;
private: System::Windows::Forms::Label^  tboxActiveSportCode;
private: System::Windows::Forms::Label^  lblActiveDataBits;
private: System::Windows::Forms::Button^  btnActiveNetwork;
private: System::Windows::Forms::Button^  btnActiveDatabase;
private: System::Windows::Forms::Label^  tboxActiveParity;
private: System::Windows::Forms::Label^  lblActiveType;
private: System::Windows::Forms::Label^  tboxActiveBaud;
private: System::Windows::Forms::Label^  tboxActivePort;
private: System::Windows::Forms::Label^  lblActiveStopBits;
private: System::Windows::Forms::Label^  tboxActiveSport;
private: System::Windows::Forms::Label^  tboxActiveBoard;
private: System::Windows::Forms::Label^  tboxActiveName;
private: System::Windows::Forms::Label^  lblActiveSerial;
private: System::Windows::Forms::Label^  lblActiveParity;
private: System::Windows::Forms::Label^  lblActiveTitle;
private: System::Windows::Forms::Label^  lblActiveName;
private: System::Windows::Forms::Label^  lblActiveBoard;
private: System::Windows::Forms::Label^  lblActiveBaud;
private: System::Windows::Forms::Label^  lblActiveSport;
private: System::Windows::Forms::Label^  lblActivePort;
private: System::Windows::Forms::PictureBox^  pictureBox2;



	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::IContainer^  components;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->components = (gcnew System::ComponentModel::Container());
		System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(DataLinkForm::typeid));
		this->panel1 = (gcnew System::Windows::Forms::Panel());
		this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
		this->panel3 = (gcnew System::Windows::Forms::Panel());
		this->chkUseText = (gcnew System::Windows::Forms::CheckBox());
		this->chkActiveDatabase = (gcnew System::Windows::Forms::CheckBox());
		this->btnActiveLiveText = (gcnew System::Windows::Forms::Button());
		this->tboxActiveDataBits = (gcnew System::Windows::Forms::Label());
		this->chkActiveNetwork = (gcnew System::Windows::Forms::CheckBox());
		this->tboxActiveStopBits = (gcnew System::Windows::Forms::Label());
		this->chkActiveSerial = (gcnew System::Windows::Forms::CheckBox());
		this->btnActiveText = (gcnew System::Windows::Forms::Button());
		this->tboxActiveSportCode = (gcnew System::Windows::Forms::Label());
		this->lblActiveDataBits = (gcnew System::Windows::Forms::Label());
		this->btnActiveNetwork = (gcnew System::Windows::Forms::Button());
		this->btnActiveDatabase = (gcnew System::Windows::Forms::Button());
		this->tboxActiveParity = (gcnew System::Windows::Forms::Label());
		this->lblActiveType = (gcnew System::Windows::Forms::Label());
		this->tboxActiveBaud = (gcnew System::Windows::Forms::Label());
		this->tboxActivePort = (gcnew System::Windows::Forms::Label());
		this->lblActiveStopBits = (gcnew System::Windows::Forms::Label());
		this->tboxActiveSport = (gcnew System::Windows::Forms::Label());
		this->tboxActiveBoard = (gcnew System::Windows::Forms::Label());
		this->tboxActiveName = (gcnew System::Windows::Forms::Label());
		this->lblActiveSerial = (gcnew System::Windows::Forms::Label());
		this->lblActiveParity = (gcnew System::Windows::Forms::Label());
		this->lblActiveTitle = (gcnew System::Windows::Forms::Label());
		this->lblActiveName = (gcnew System::Windows::Forms::Label());
		this->lblActiveBoard = (gcnew System::Windows::Forms::Label());
		this->lblActiveBaud = (gcnew System::Windows::Forms::Label());
		this->lblActiveSport = (gcnew System::Windows::Forms::Label());
		this->lblActivePort = (gcnew System::Windows::Forms::Label());
		this->panel2 = (gcnew System::Windows::Forms::Panel());
		this->cmbSettingsParity = (gcnew System::Windows::Forms::ComboBox());
		this->cmbSettingsStopBits = (gcnew System::Windows::Forms::ComboBox());
		this->cmbSettingsDataBits = (gcnew System::Windows::Forms::ComboBox());
		this->chkSettingsDatabase = (gcnew System::Windows::Forms::CheckBox());
		this->chkSettingsNetwork = (gcnew System::Windows::Forms::CheckBox());
		this->chkSettingsSerial = (gcnew System::Windows::Forms::CheckBox());
		this->cmbSettingsBoard = (gcnew System::Windows::Forms::ComboBox());
		this->lblSettingsParity = (gcnew System::Windows::Forms::Label());
		this->cmbSettingsName = (gcnew System::Windows::Forms::ComboBox());
		this->lblSettingsDataBits = (gcnew System::Windows::Forms::Label());
		this->lblSettingsType = (gcnew System::Windows::Forms::Label());
		this->lblSettingsPort = (gcnew System::Windows::Forms::Label());
		this->btnSave = (gcnew System::Windows::Forms::Button());
		this->lblSettingsStopBits = (gcnew System::Windows::Forms::Label());
		this->lblSettingsSport = (gcnew System::Windows::Forms::Label());
		this->lblSettingsBaud = (gcnew System::Windows::Forms::Label());
		this->lblSettingsBoard = (gcnew System::Windows::Forms::Label());
		this->btnSettingsNetwork = (gcnew System::Windows::Forms::Button());
		this->lblSettingsName = (gcnew System::Windows::Forms::Label());
		this->cmbSettingsBaud = (gcnew System::Windows::Forms::ComboBox());
		this->cmbSettingsSportCode = (gcnew System::Windows::Forms::ComboBox());
		this->lblSettingsSerial = (gcnew System::Windows::Forms::Label());
		this->lblSettingsTitle = (gcnew System::Windows::Forms::Label());
		this->cmbSettingsPort = (gcnew System::Windows::Forms::ComboBox());
		this->btnDelete = (gcnew System::Windows::Forms::Button());
		this->btnNew = (gcnew System::Windows::Forms::Button());
		this->btnSettingsDatabase = (gcnew System::Windows::Forms::Button());
		this->cmbSettingsSport = (gcnew System::Windows::Forms::ComboBox());
		this->panel4 = (gcnew System::Windows::Forms::Panel());
		this->linkNewTek = (gcnew System::Windows::Forms::LinkLabel());
		this->lblCopyright = (gcnew System::Windows::Forms::Label());
		this->btnExit = (gcnew System::Windows::Forms::Button());
		this->btnClearActive = (gcnew System::Windows::Forms::Button());
		this->btnActivate = (gcnew System::Windows::Forms::Button());
		this->fadeInTimer = (gcnew System::Windows::Forms::Timer(this->components));
		this->fadeOutTimer = (gcnew System::Windows::Forms::Timer(this->components));
		this->panel1->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->BeginInit();
		this->panel3->SuspendLayout();
		this->panel2->SuspendLayout();
		this->panel4->SuspendLayout();
		this->SuspendLayout();
		// 
		// panel1
		// 
		this->panel1->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel1.BackgroundImage")));
		this->panel1->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel1->Controls->Add(this->pictureBox2);
		this->panel1->Controls->Add(this->panel3);
		this->panel1->Controls->Add(this->panel2);
		this->panel1->Controls->Add(this->panel4);
		this->panel1->Controls->Add(this->btnExit);
		this->panel1->Controls->Add(this->btnClearActive);
		this->panel1->Controls->Add(this->btnActivate);
		this->panel1->Location = System::Drawing::Point(0, 0);
		this->panel1->Name = L"panel1";
		this->panel1->Size = System::Drawing::Size(541, 486);
		this->panel1->TabIndex = 0;
		// 
		// pictureBox2
		// 
		this->pictureBox2->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox2.BackgroundImage")));
		this->pictureBox2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox2.Image")));
		this->pictureBox2->Location = System::Drawing::Point(-2, -2);
		this->pictureBox2->Name = L"pictureBox2";
		this->pictureBox2->Size = System::Drawing::Size(541, 98);
		this->pictureBox2->TabIndex = 100;
		this->pictureBox2->TabStop = false;
		// 
		// panel3
		// 
		this->panel3->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel3.BackgroundImage")));
		this->panel3->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->panel3->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel3->Controls->Add(this->chkUseText);
		this->panel3->Controls->Add(this->chkActiveDatabase);
		this->panel3->Controls->Add(this->btnActiveLiveText);
		this->panel3->Controls->Add(this->tboxActiveDataBits);
		this->panel3->Controls->Add(this->chkActiveNetwork);
		this->panel3->Controls->Add(this->tboxActiveStopBits);
		this->panel3->Controls->Add(this->chkActiveSerial);
		this->panel3->Controls->Add(this->btnActiveText);
		this->panel3->Controls->Add(this->tboxActiveSportCode);
		this->panel3->Controls->Add(this->lblActiveDataBits);
		this->panel3->Controls->Add(this->btnActiveNetwork);
		this->panel3->Controls->Add(this->btnActiveDatabase);
		this->panel3->Controls->Add(this->tboxActiveParity);
		this->panel3->Controls->Add(this->lblActiveType);
		this->panel3->Controls->Add(this->tboxActiveBaud);
		this->panel3->Controls->Add(this->tboxActivePort);
		this->panel3->Controls->Add(this->lblActiveStopBits);
		this->panel3->Controls->Add(this->tboxActiveSport);
		this->panel3->Controls->Add(this->tboxActiveBoard);
		this->panel3->Controls->Add(this->tboxActiveName);
		this->panel3->Controls->Add(this->lblActiveSerial);
		this->panel3->Controls->Add(this->lblActiveParity);
		this->panel3->Controls->Add(this->lblActiveTitle);
		this->panel3->Controls->Add(this->lblActiveName);
		this->panel3->Controls->Add(this->lblActiveBoard);
		this->panel3->Controls->Add(this->lblActiveBaud);
		this->panel3->Controls->Add(this->lblActiveSport);
		this->panel3->Controls->Add(this->lblActivePort);
		this->panel3->Location = System::Drawing::Point(273, 102);
		this->panel3->Name = L"panel3";
		this->panel3->Size = System::Drawing::Size(260, 348);
		this->panel3->TabIndex = 2;
		// 
		// chkUseText
		// 
		this->chkUseText->AutoSize = true;
		this->chkUseText->BackColor = System::Drawing::Color::Transparent;
		this->chkUseText->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->chkUseText->Location = System::Drawing::Point(8, 324);
		this->chkUseText->Name = L"chkUseText";
		this->chkUseText->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->chkUseText->Size = System::Drawing::Size(234, 17);
		this->chkUseText->TabIndex = 0;
		this->chkUseText->TabStop = false;
		this->chkUseText->Text = L"Use text input (from \"..\\Stats Plugins\" folder)";
		this->chkUseText->UseVisualStyleBackColor = false;
		this->chkUseText->Visible = false;
		this->chkUseText->CheckedChanged += gcnew System::EventHandler(this, &DataLinkForm::chkUseText_CheckedChanged);
		// 
		// chkActiveDatabase
		// 
		this->chkActiveDatabase->AutoCheck = false;
		this->chkActiveDatabase->AutoSize = true;
		this->chkActiveDatabase->BackColor = System::Drawing::Color::Transparent;
		this->chkActiveDatabase->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, 
			System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
		this->chkActiveDatabase->Location = System::Drawing::Point(56, 142);
		this->chkActiveDatabase->Name = L"chkActiveDatabase";
		this->chkActiveDatabase->Size = System::Drawing::Size(15, 14);
		this->chkActiveDatabase->TabIndex = 0;
		this->chkActiveDatabase->TabStop = false;
		this->chkActiveDatabase->UseVisualStyleBackColor = false;
		// 
		// btnActiveLiveText
		// 
		this->btnActiveLiveText->BackColor = System::Drawing::Color::Transparent;
		this->btnActiveLiveText->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnActiveLiveText.BackgroundImage")));
		this->btnActiveLiveText->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
		this->btnActiveLiveText->FlatAppearance->BorderColor = System::Drawing::Color::White;
		this->btnActiveLiveText->FlatAppearance->BorderSize = 0;
		this->btnActiveLiveText->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Transparent;
		this->btnActiveLiveText->FlatAppearance->MouseOverBackColor = System::Drawing::Color::Transparent;
		this->btnActiveLiveText->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnActiveLiveText->ForeColor = System::Drawing::Color::Transparent;
		this->btnActiveLiveText->Location = System::Drawing::Point(189, 139);
		this->btnActiveLiveText->Margin = System::Windows::Forms::Padding(0);
		this->btnActiveLiveText->Name = L"btnActiveLiveText";
		this->btnActiveLiveText->Size = System::Drawing::Size(42, 40);
		this->btnActiveLiveText->TabIndex = 0;
		this->btnActiveLiveText->TabStop = false;
		this->btnActiveLiveText->TextImageRelation = System::Windows::Forms::TextImageRelation::ImageBeforeText;
		this->btnActiveLiveText->UseVisualStyleBackColor = false;
		this->btnActiveLiveText->Click += gcnew System::EventHandler(this, &DataLinkForm::btnActiveLiveText_Click);
		// 
		// tboxActiveDataBits
		// 
		this->tboxActiveDataBits->AutoSize = true;
		this->tboxActiveDataBits->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveDataBits->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveDataBits->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveDataBits->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveDataBits->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveDataBits->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveDataBits->Location = System::Drawing::Point(199, 244);
		this->tboxActiveDataBits->MinimumSize = System::Drawing::Size(47, 23);
		this->tboxActiveDataBits->Name = L"tboxActiveDataBits";
		this->tboxActiveDataBits->Size = System::Drawing::Size(47, 23);
		this->tboxActiveDataBits->TabIndex = 0;
		this->tboxActiveDataBits->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// chkActiveNetwork
		// 
		this->chkActiveNetwork->AutoCheck = false;
		this->chkActiveNetwork->AutoSize = true;
		this->chkActiveNetwork->BackColor = System::Drawing::Color::Transparent;
		this->chkActiveNetwork->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, 
			System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
		this->chkActiveNetwork->Location = System::Drawing::Point(56, 114);
		this->chkActiveNetwork->Name = L"chkActiveNetwork";
		this->chkActiveNetwork->Size = System::Drawing::Size(15, 14);
		this->chkActiveNetwork->TabIndex = 0;
		this->chkActiveNetwork->TabStop = false;
		this->chkActiveNetwork->UseVisualStyleBackColor = false;
		// 
		// tboxActiveStopBits
		// 
		this->tboxActiveStopBits->AutoSize = true;
		this->tboxActiveStopBits->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveStopBits->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveStopBits->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveStopBits->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveStopBits->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveStopBits->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveStopBits->Location = System::Drawing::Point(199, 300);
		this->tboxActiveStopBits->MinimumSize = System::Drawing::Size(47, 23);
		this->tboxActiveStopBits->Name = L"tboxActiveStopBits";
		this->tboxActiveStopBits->Size = System::Drawing::Size(47, 23);
		this->tboxActiveStopBits->TabIndex = 0;
		this->tboxActiveStopBits->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// chkActiveSerial
		// 
		this->chkActiveSerial->AutoCheck = false;
		this->chkActiveSerial->AutoSize = true;
		this->chkActiveSerial->BackColor = System::Drawing::Color::Transparent;
		this->chkActiveSerial->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->chkActiveSerial->Location = System::Drawing::Point(56, 170);
		this->chkActiveSerial->Name = L"chkActiveSerial";
		this->chkActiveSerial->Size = System::Drawing::Size(15, 14);
		this->chkActiveSerial->TabIndex = 0;
		this->chkActiveSerial->TabStop = false;
		this->chkActiveSerial->UseVisualStyleBackColor = false;
		// 
		// btnActiveText
		// 
		this->btnActiveText->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnActiveText.BackgroundImage")));
		this->btnActiveText->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnActiveText->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnActiveText->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnActiveText->Location = System::Drawing::Point(173, 108);
		this->btnActiveText->Name = L"btnActiveText";
		this->btnActiveText->Size = System::Drawing::Size(73, 23);
		this->btnActiveText->TabIndex = 0;
		this->btnActiveText->TabStop = false;
		this->btnActiveText->Text = L"Text Folder";
		this->btnActiveText->UseVisualStyleBackColor = true;
		this->btnActiveText->Click += gcnew System::EventHandler(this, &DataLinkForm::btnActiveText_Click);
		// 
		// tboxActiveSportCode
		// 
		this->tboxActiveSportCode->AutoSize = true;
		this->tboxActiveSportCode->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveSportCode->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveSportCode->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveSportCode->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveSportCode->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveSportCode->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveSportCode->Location = System::Drawing::Point(179, 216);
		this->tboxActiveSportCode->MinimumSize = System::Drawing::Size(67, 23);
		this->tboxActiveSportCode->Name = L"tboxActiveSportCode";
		this->tboxActiveSportCode->Size = System::Drawing::Size(67, 23);
		this->tboxActiveSportCode->TabIndex = 0;
		this->tboxActiveSportCode->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// lblActiveDataBits
		// 
		this->lblActiveDataBits->AutoSize = true;
		this->lblActiveDataBits->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveDataBits->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveDataBits->Location = System::Drawing::Point(130, 244);
		this->lblActiveDataBits->Name = L"lblActiveDataBits";
		this->lblActiveDataBits->Size = System::Drawing::Size(71, 21);
		this->lblActiveDataBits->TabIndex = 0;
		this->lblActiveDataBits->Text = L"Data Bits";
		this->lblActiveDataBits->TextAlign = System::Drawing::ContentAlignment::TopRight;
		// 
		// btnActiveNetwork
		// 
		this->btnActiveNetwork->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnActiveNetwork.BackgroundImage")));
		this->btnActiveNetwork->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnActiveNetwork->Enabled = false;
		this->btnActiveNetwork->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnActiveNetwork->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnActiveNetwork->Location = System::Drawing::Point(78, 108);
		this->btnActiveNetwork->Name = L"btnActiveNetwork";
		this->btnActiveNetwork->Size = System::Drawing::Size(73, 23);
		this->btnActiveNetwork->TabIndex = 0;
		this->btnActiveNetwork->TabStop = false;
		this->btnActiveNetwork->Text = L"Network";
		this->btnActiveNetwork->UseVisualStyleBackColor = true;
		this->btnActiveNetwork->Click += gcnew System::EventHandler(this, &DataLinkForm::btnActiveNetwork_Click);
		// 
		// btnActiveDatabase
		// 
		this->btnActiveDatabase->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnActiveDatabase.BackgroundImage")));
		this->btnActiveDatabase->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnActiveDatabase->Enabled = false;
		this->btnActiveDatabase->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnActiveDatabase->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnActiveDatabase->Location = System::Drawing::Point(78, 136);
		this->btnActiveDatabase->Name = L"btnActiveDatabase";
		this->btnActiveDatabase->Size = System::Drawing::Size(73, 23);
		this->btnActiveDatabase->TabIndex = 0;
		this->btnActiveDatabase->TabStop = false;
		this->btnActiveDatabase->Text = L"Database";
		this->btnActiveDatabase->UseVisualStyleBackColor = true;
		this->btnActiveDatabase->Click += gcnew System::EventHandler(this, &DataLinkForm::btnActiveDatabase_Click);
		// 
		// tboxActiveParity
		// 
		this->tboxActiveParity->AutoSize = true;
		this->tboxActiveParity->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveParity->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveParity->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveParity->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveParity->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveParity->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveParity->Location = System::Drawing::Point(199, 272);
		this->tboxActiveParity->MinimumSize = System::Drawing::Size(47, 23);
		this->tboxActiveParity->Name = L"tboxActiveParity";
		this->tboxActiveParity->Size = System::Drawing::Size(47, 23);
		this->tboxActiveParity->TabIndex = 0;
		this->tboxActiveParity->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// lblActiveType
		// 
		this->lblActiveType->AutoSize = true;
		this->lblActiveType->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveType->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveType->Location = System::Drawing::Point(3, 111);
		this->lblActiveType->Name = L"lblActiveType";
		this->lblActiveType->Size = System::Drawing::Size(50, 21);
		this->lblActiveType->TabIndex = 0;
		this->lblActiveType->Text = L"Types";
		// 
		// tboxActiveBaud
		// 
		this->tboxActiveBaud->AutoSize = true;
		this->tboxActiveBaud->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveBaud->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveBaud->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveBaud->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveBaud->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveBaud->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveBaud->Location = System::Drawing::Point(56, 272);
		this->tboxActiveBaud->MinimumSize = System::Drawing::Size(53, 23);
		this->tboxActiveBaud->Name = L"tboxActiveBaud";
		this->tboxActiveBaud->Size = System::Drawing::Size(53, 23);
		this->tboxActiveBaud->TabIndex = 0;
		this->tboxActiveBaud->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// tboxActivePort
		// 
		this->tboxActivePort->AutoSize = true;
		this->tboxActivePort->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActivePort->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActivePort->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActivePort->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActivePort->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActivePort->ForeColor = System::Drawing::Color::Black;
		this->tboxActivePort->Location = System::Drawing::Point(56, 244);
		this->tboxActivePort->MinimumSize = System::Drawing::Size(53, 23);
		this->tboxActivePort->Name = L"tboxActivePort";
		this->tboxActivePort->Size = System::Drawing::Size(53, 23);
		this->tboxActivePort->TabIndex = 0;
		this->tboxActivePort->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		// 
		// lblActiveStopBits
		// 
		this->lblActiveStopBits->AutoSize = true;
		this->lblActiveStopBits->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveStopBits->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveStopBits->Location = System::Drawing::Point(128, 300);
		this->lblActiveStopBits->Name = L"lblActiveStopBits";
		this->lblActiveStopBits->Size = System::Drawing::Size(71, 21);
		this->lblActiveStopBits->TabIndex = 0;
		this->lblActiveStopBits->Text = L"Stop Bits";
		this->lblActiveStopBits->TextAlign = System::Drawing::ContentAlignment::TopRight;
		// 
		// tboxActiveSport
		// 
		this->tboxActiveSport->AutoSize = true;
		this->tboxActiveSport->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveSport->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveSport->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveSport->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveSport->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveSport->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveSport->Location = System::Drawing::Point(56, 216);
		this->tboxActiveSport->MinimumSize = System::Drawing::Size(116, 23);
		this->tboxActiveSport->Name = L"tboxActiveSport";
		this->tboxActiveSport->Size = System::Drawing::Size(116, 23);
		this->tboxActiveSport->TabIndex = 0;
		this->tboxActiveSport->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
		// 
		// tboxActiveBoard
		// 
		this->tboxActiveBoard->AutoSize = true;
		this->tboxActiveBoard->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveBoard->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveBoard->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveBoard->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveBoard->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveBoard->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveBoard->Location = System::Drawing::Point(56, 188);
		this->tboxActiveBoard->MinimumSize = System::Drawing::Size(190, 23);
		this->tboxActiveBoard->Name = L"tboxActiveBoard";
		this->tboxActiveBoard->Size = System::Drawing::Size(190, 23);
		this->tboxActiveBoard->TabIndex = 0;
		this->tboxActiveBoard->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
		// 
		// tboxActiveName
		// 
		this->tboxActiveName->AutoSize = true;
		this->tboxActiveName->BackColor = System::Drawing::SystemColors::Window;
		this->tboxActiveName->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->tboxActiveName->Cursor = System::Windows::Forms::Cursors::Arrow;
		this->tboxActiveName->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		this->tboxActiveName->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->tboxActiveName->ForeColor = System::Drawing::Color::Black;
		this->tboxActiveName->Location = System::Drawing::Point(56, 78);
		this->tboxActiveName->MaximumSize = System::Drawing::Size(190, 23);
		this->tboxActiveName->MinimumSize = System::Drawing::Size(190, 23);
		this->tboxActiveName->Name = L"tboxActiveName";
		this->tboxActiveName->Size = System::Drawing::Size(190, 23);
		this->tboxActiveName->TabIndex = 0;
		this->tboxActiveName->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
		// 
		// lblActiveSerial
		// 
		this->lblActiveSerial->AutoSize = true;
		this->lblActiveSerial->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveSerial->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveSerial->Location = System::Drawing::Point(76, 164);
		this->lblActiveSerial->Name = L"lblActiveSerial";
		this->lblActiveSerial->Size = System::Drawing::Size(49, 21);
		this->lblActiveSerial->TabIndex = 0;
		this->lblActiveSerial->Text = L"Serial";
		// 
		// lblActiveParity
		// 
		this->lblActiveParity->AutoSize = true;
		this->lblActiveParity->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveParity->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveParity->Location = System::Drawing::Point(150, 272);
		this->lblActiveParity->Name = L"lblActiveParity";
		this->lblActiveParity->Size = System::Drawing::Size(50, 21);
		this->lblActiveParity->TabIndex = 0;
		this->lblActiveParity->Text = L"Parity";
		// 
		// lblActiveTitle
		// 
		this->lblActiveTitle->AutoSize = true;
		this->lblActiveTitle->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 14.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveTitle->Location = System::Drawing::Point(54, 11);
		this->lblActiveTitle->Name = L"lblActiveTitle";
		this->lblActiveTitle->Size = System::Drawing::Size(150, 25);
		this->lblActiveTitle->TabIndex = 0;
		this->lblActiveTitle->Text = L"Active Settings";
		// 
		// lblActiveName
		// 
		this->lblActiveName->AutoSize = true;
		this->lblActiveName->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveName->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveName->Location = System::Drawing::Point(4, 80);
		this->lblActiveName->Name = L"lblActiveName";
		this->lblActiveName->Size = System::Drawing::Size(52, 21);
		this->lblActiveName->TabIndex = 0;
		this->lblActiveName->Text = L"Name";
		// 
		// lblActiveBoard
		// 
		this->lblActiveBoard->AutoSize = true;
		this->lblActiveBoard->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveBoard->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveBoard->Location = System::Drawing::Point(4, 188);
		this->lblActiveBoard->Name = L"lblActiveBoard";
		this->lblActiveBoard->Size = System::Drawing::Size(51, 21);
		this->lblActiveBoard->TabIndex = 0;
		this->lblActiveBoard->Text = L"Board";
		// 
		// lblActiveBaud
		// 
		this->lblActiveBaud->AutoSize = true;
		this->lblActiveBaud->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveBaud->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveBaud->Location = System::Drawing::Point(4, 272);
		this->lblActiveBaud->Name = L"lblActiveBaud";
		this->lblActiveBaud->Size = System::Drawing::Size(45, 21);
		this->lblActiveBaud->TabIndex = 0;
		this->lblActiveBaud->Text = L"Baud";
		// 
		// lblActiveSport
		// 
		this->lblActiveSport->AutoSize = true;
		this->lblActiveSport->BackColor = System::Drawing::Color::Transparent;
		this->lblActiveSport->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActiveSport->Location = System::Drawing::Point(4, 216);
		this->lblActiveSport->Name = L"lblActiveSport";
		this->lblActiveSport->Size = System::Drawing::Size(48, 21);
		this->lblActiveSport->TabIndex = 0;
		this->lblActiveSport->Text = L"Sport";
		// 
		// lblActivePort
		// 
		this->lblActivePort->AutoSize = true;
		this->lblActivePort->BackColor = System::Drawing::Color::Transparent;
		this->lblActivePort->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblActivePort->Location = System::Drawing::Point(4, 244);
		this->lblActivePort->Name = L"lblActivePort";
		this->lblActivePort->Size = System::Drawing::Size(39, 21);
		this->lblActivePort->TabIndex = 0;
		this->lblActivePort->Text = L"Port";
		// 
		// panel2
		// 
		this->panel2->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"panel2.BackgroundImage")));
		this->panel2->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
		this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
		this->panel2->Controls->Add(this->cmbSettingsParity);
		this->panel2->Controls->Add(this->cmbSettingsStopBits);
		this->panel2->Controls->Add(this->cmbSettingsDataBits);
		this->panel2->Controls->Add(this->chkSettingsDatabase);
		this->panel2->Controls->Add(this->chkSettingsNetwork);
		this->panel2->Controls->Add(this->chkSettingsSerial);
		this->panel2->Controls->Add(this->cmbSettingsBoard);
		this->panel2->Controls->Add(this->lblSettingsParity);
		this->panel2->Controls->Add(this->cmbSettingsName);
		this->panel2->Controls->Add(this->lblSettingsDataBits);
		this->panel2->Controls->Add(this->lblSettingsType);
		this->panel2->Controls->Add(this->lblSettingsPort);
		this->panel2->Controls->Add(this->btnSave);
		this->panel2->Controls->Add(this->lblSettingsStopBits);
		this->panel2->Controls->Add(this->lblSettingsSport);
		this->panel2->Controls->Add(this->lblSettingsBaud);
		this->panel2->Controls->Add(this->lblSettingsBoard);
		this->panel2->Controls->Add(this->btnSettingsNetwork);
		this->panel2->Controls->Add(this->lblSettingsName);
		this->panel2->Controls->Add(this->cmbSettingsBaud);
		this->panel2->Controls->Add(this->cmbSettingsSportCode);
		this->panel2->Controls->Add(this->lblSettingsSerial);
		this->panel2->Controls->Add(this->lblSettingsTitle);
		this->panel2->Controls->Add(this->cmbSettingsPort);
		this->panel2->Controls->Add(this->btnDelete);
		this->panel2->Controls->Add(this->btnNew);
		this->panel2->Controls->Add(this->btnSettingsDatabase);
		this->panel2->Controls->Add(this->cmbSettingsSport);
		this->panel2->Location = System::Drawing::Point(4, 102);
		this->panel2->Name = L"panel2";
		this->panel2->Size = System::Drawing::Size(260, 348);
		this->panel2->TabIndex = 1;
		// 
		// cmbSettingsParity
		// 
		this->cmbSettingsParity->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsParity->Enabled = false;
		this->cmbSettingsParity->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsParity->FormattingEnabled = true;
		this->cmbSettingsParity->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"", L"0", L"1", L"2", L"3", L"4"});
		this->cmbSettingsParity->Location = System::Drawing::Point(199, 272);
		this->cmbSettingsParity->MaxDropDownItems = 5;
		this->cmbSettingsParity->Name = L"cmbSettingsParity";
		this->cmbSettingsParity->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->cmbSettingsParity->Size = System::Drawing::Size(47, 23);
		this->cmbSettingsParity->TabIndex = 0;
		this->cmbSettingsParity->TabStop = false;
		this->cmbSettingsParity->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsParity_SelectedIndexChanged);
		// 
		// cmbSettingsStopBits
		// 
		this->cmbSettingsStopBits->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsStopBits->Enabled = false;
		this->cmbSettingsStopBits->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsStopBits->FormattingEnabled = true;
		this->cmbSettingsStopBits->Items->AddRange(gcnew cli::array< System::Object^  >(4) {L"", L"0", L"1", L"2"});
		this->cmbSettingsStopBits->Location = System::Drawing::Point(199, 300);
		this->cmbSettingsStopBits->MaxDropDownItems = 5;
		this->cmbSettingsStopBits->Name = L"cmbSettingsStopBits";
		this->cmbSettingsStopBits->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->cmbSettingsStopBits->Size = System::Drawing::Size(47, 23);
		this->cmbSettingsStopBits->TabIndex = 0;
		this->cmbSettingsStopBits->TabStop = false;
		this->cmbSettingsStopBits->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsStopBits_SelectedIndexChanged);
		// 
		// cmbSettingsDataBits
		// 
		this->cmbSettingsDataBits->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsDataBits->Enabled = false;
		this->cmbSettingsDataBits->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsDataBits->FormattingEnabled = true;
		this->cmbSettingsDataBits->Items->AddRange(gcnew cli::array< System::Object^  >(5) {L"", L"5", L"6", L"7", L"8"});
		this->cmbSettingsDataBits->Location = System::Drawing::Point(199, 244);
		this->cmbSettingsDataBits->MaxDropDownItems = 5;
		this->cmbSettingsDataBits->Name = L"cmbSettingsDataBits";
		this->cmbSettingsDataBits->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->cmbSettingsDataBits->Size = System::Drawing::Size(47, 23);
		this->cmbSettingsDataBits->TabIndex = 0;
		this->cmbSettingsDataBits->TabStop = false;
		this->cmbSettingsDataBits->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsDataBits_SelectedIndexChanged);
		// 
		// chkSettingsDatabase
		// 
		this->chkSettingsDatabase->AutoSize = true;
		this->chkSettingsDatabase->BackColor = System::Drawing::Color::Transparent;
		this->chkSettingsDatabase->Location = System::Drawing::Point(56, 142);
		this->chkSettingsDatabase->Name = L"chkSettingsDatabase";
		this->chkSettingsDatabase->Size = System::Drawing::Size(15, 14);
		this->chkSettingsDatabase->TabIndex = 0;
		this->chkSettingsDatabase->TabStop = false;
		this->chkSettingsDatabase->UseVisualStyleBackColor = false;
		this->chkSettingsDatabase->CheckedChanged += gcnew System::EventHandler(this, &DataLinkForm::chkSettingsDatabase_CheckedChanged);
		// 
		// chkSettingsNetwork
		// 
		this->chkSettingsNetwork->AutoSize = true;
		this->chkSettingsNetwork->BackColor = System::Drawing::Color::Transparent;
		this->chkSettingsNetwork->Location = System::Drawing::Point(56, 114);
		this->chkSettingsNetwork->Name = L"chkSettingsNetwork";
		this->chkSettingsNetwork->Size = System::Drawing::Size(15, 14);
		this->chkSettingsNetwork->TabIndex = 0;
		this->chkSettingsNetwork->TabStop = false;
		this->chkSettingsNetwork->UseVisualStyleBackColor = false;
		this->chkSettingsNetwork->CheckedChanged += gcnew System::EventHandler(this, &DataLinkForm::chkSettingsNetwork_CheckedChanged);
		// 
		// chkSettingsSerial
		// 
		this->chkSettingsSerial->AutoSize = true;
		this->chkSettingsSerial->BackColor = System::Drawing::Color::Transparent;
		this->chkSettingsSerial->Location = System::Drawing::Point(56, 170);
		this->chkSettingsSerial->Name = L"chkSettingsSerial";
		this->chkSettingsSerial->Size = System::Drawing::Size(15, 14);
		this->chkSettingsSerial->TabIndex = 0;
		this->chkSettingsSerial->TabStop = false;
		this->chkSettingsSerial->UseVisualStyleBackColor = false;
		this->chkSettingsSerial->CheckedChanged += gcnew System::EventHandler(this, &DataLinkForm::chkSettingsSerial_CheckedChanged);
		// 
		// cmbSettingsBoard
		// 
		this->cmbSettingsBoard->AutoCompleteMode = System::Windows::Forms::AutoCompleteMode::SuggestAppend;
		this->cmbSettingsBoard->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::ListItems;
		this->cmbSettingsBoard->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsBoard->Enabled = false;
		this->cmbSettingsBoard->FlatStyle = System::Windows::Forms::FlatStyle::System;
		this->cmbSettingsBoard->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsBoard->FormattingEnabled = true;
		this->cmbSettingsBoard->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L""});
		this->cmbSettingsBoard->Location = System::Drawing::Point(56, 188);
		this->cmbSettingsBoard->MaxDropDownItems = 5;
		this->cmbSettingsBoard->Name = L"cmbSettingsBoard";
		this->cmbSettingsBoard->Size = System::Drawing::Size(190, 23);
		this->cmbSettingsBoard->TabIndex = 0;
		this->cmbSettingsBoard->TabStop = false;
		this->cmbSettingsBoard->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsBoard_SelectedIndexChanged);
		// 
		// lblSettingsParity
		// 
		this->lblSettingsParity->AutoSize = true;
		this->lblSettingsParity->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsParity->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsParity->Location = System::Drawing::Point(132, 272);
		this->lblSettingsParity->Name = L"lblSettingsParity";
		this->lblSettingsParity->Size = System::Drawing::Size(50, 21);
		this->lblSettingsParity->TabIndex = 0;
		this->lblSettingsParity->Text = L"Parity";
		// 
		// cmbSettingsName
		// 
		this->cmbSettingsName->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsName->FormattingEnabled = true;
		this->cmbSettingsName->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L""});
		this->cmbSettingsName->Location = System::Drawing::Point(56, 78);
		this->cmbSettingsName->MaxLength = 24;
		this->cmbSettingsName->Name = L"cmbSettingsName";
		this->cmbSettingsName->Size = System::Drawing::Size(190, 23);
		this->cmbSettingsName->TabIndex = 0;
		this->cmbSettingsName->TabStop = false;
		this->cmbSettingsName->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsName_SelectedIndexChanged);
		this->cmbSettingsName->TextChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsName_TextChanged);
		// 
		// lblSettingsDataBits
		// 
		this->lblSettingsDataBits->AutoSize = true;
		this->lblSettingsDataBits->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsDataBits->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsDataBits->Location = System::Drawing::Point(132, 244);
		this->lblSettingsDataBits->Name = L"lblSettingsDataBits";
		this->lblSettingsDataBits->Size = System::Drawing::Size(71, 21);
		this->lblSettingsDataBits->TabIndex = 0;
		this->lblSettingsDataBits->Text = L"Data Bits";
		this->lblSettingsDataBits->TextAlign = System::Drawing::ContentAlignment::TopRight;
		// 
		// lblSettingsType
		// 
		this->lblSettingsType->AutoSize = true;
		this->lblSettingsType->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsType->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsType->Location = System::Drawing::Point(3, 108);
		this->lblSettingsType->Name = L"lblSettingsType";
		this->lblSettingsType->Size = System::Drawing::Size(50, 21);
		this->lblSettingsType->TabIndex = 0;
		this->lblSettingsType->Text = L"Types";
		// 
		// lblSettingsPort
		// 
		this->lblSettingsPort->AutoSize = true;
		this->lblSettingsPort->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsPort->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsPort->Location = System::Drawing::Point(3, 244);
		this->lblSettingsPort->Name = L"lblSettingsPort";
		this->lblSettingsPort->Size = System::Drawing::Size(39, 21);
		this->lblSettingsPort->TabIndex = 0;
		this->lblSettingsPort->Text = L"Port";
		// 
		// btnSave
		// 
		this->btnSave->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSave.BackgroundImage")));
		this->btnSave->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnSave->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnSave->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnSave->Location = System::Drawing::Point(121, 45);
		this->btnSave->Name = L"btnSave";
		this->btnSave->Size = System::Drawing::Size(61, 23);
		this->btnSave->TabIndex = 0;
		this->btnSave->TabStop = false;
		this->btnSave->Text = L"Save";
		this->btnSave->UseVisualStyleBackColor = true;
		this->btnSave->Click += gcnew System::EventHandler(this, &DataLinkForm::btnSave_Click);
		// 
		// lblSettingsStopBits
		// 
		this->lblSettingsStopBits->AutoSize = true;
		this->lblSettingsStopBits->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsStopBits->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsStopBits->Location = System::Drawing::Point(132, 300);
		this->lblSettingsStopBits->Name = L"lblSettingsStopBits";
		this->lblSettingsStopBits->Size = System::Drawing::Size(71, 21);
		this->lblSettingsStopBits->TabIndex = 0;
		this->lblSettingsStopBits->Text = L"Stop Bits";
		this->lblSettingsStopBits->TextAlign = System::Drawing::ContentAlignment::TopRight;
		// 
		// lblSettingsSport
		// 
		this->lblSettingsSport->AutoSize = true;
		this->lblSettingsSport->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsSport->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsSport->Location = System::Drawing::Point(3, 216);
		this->lblSettingsSport->Name = L"lblSettingsSport";
		this->lblSettingsSport->Size = System::Drawing::Size(48, 21);
		this->lblSettingsSport->TabIndex = 0;
		this->lblSettingsSport->Text = L"Sport";
		// 
		// lblSettingsBaud
		// 
		this->lblSettingsBaud->AutoSize = true;
		this->lblSettingsBaud->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsBaud->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsBaud->Location = System::Drawing::Point(3, 272);
		this->lblSettingsBaud->Name = L"lblSettingsBaud";
		this->lblSettingsBaud->Size = System::Drawing::Size(45, 21);
		this->lblSettingsBaud->TabIndex = 0;
		this->lblSettingsBaud->Text = L"Baud";
		// 
		// lblSettingsBoard
		// 
		this->lblSettingsBoard->AutoSize = true;
		this->lblSettingsBoard->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsBoard->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsBoard->Location = System::Drawing::Point(3, 188);
		this->lblSettingsBoard->Name = L"lblSettingsBoard";
		this->lblSettingsBoard->Size = System::Drawing::Size(51, 21);
		this->lblSettingsBoard->TabIndex = 0;
		this->lblSettingsBoard->Text = L"Board";
		// 
		// btnSettingsNetwork
		// 
		this->btnSettingsNetwork->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSettingsNetwork.BackgroundImage")));
		this->btnSettingsNetwork->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnSettingsNetwork->Enabled = false;
		this->btnSettingsNetwork->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnSettingsNetwork->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnSettingsNetwork->Location = System::Drawing::Point(78, 108);
		this->btnSettingsNetwork->Name = L"btnSettingsNetwork";
		this->btnSettingsNetwork->Size = System::Drawing::Size(73, 23);
		this->btnSettingsNetwork->TabIndex = 0;
		this->btnSettingsNetwork->TabStop = false;
		this->btnSettingsNetwork->Text = L"Network";
		this->btnSettingsNetwork->UseVisualStyleBackColor = true;
		this->btnSettingsNetwork->Click += gcnew System::EventHandler(this, &DataLinkForm::btnSettingsNetwork_Click);
		// 
		// lblSettingsName
		// 
		this->lblSettingsName->AutoSize = true;
		this->lblSettingsName->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsName->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsName->Location = System::Drawing::Point(3, 78);
		this->lblSettingsName->Name = L"lblSettingsName";
		this->lblSettingsName->Size = System::Drawing::Size(52, 21);
		this->lblSettingsName->TabIndex = 0;
		this->lblSettingsName->Text = L"Name";
		// 
		// cmbSettingsBaud
		// 
		this->cmbSettingsBaud->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsBaud->Enabled = false;
		this->cmbSettingsBaud->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsBaud->FormattingEnabled = true;
		this->cmbSettingsBaud->Items->AddRange(gcnew cli::array< System::Object^  >(16) {L"", L"110", L"300", L"600", L"1200", L"2400", 
			L"4800", L"9600", L"14400", L"19200", L"38400", L"56000", L"57600", L"115200", L"128000", L"256000"});
		this->cmbSettingsBaud->Location = System::Drawing::Point(56, 272);
		this->cmbSettingsBaud->MaxDropDownItems = 5;
		this->cmbSettingsBaud->Name = L"cmbSettingsBaud";
		this->cmbSettingsBaud->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->cmbSettingsBaud->Size = System::Drawing::Size(64, 23);
		this->cmbSettingsBaud->TabIndex = 0;
		this->cmbSettingsBaud->TabStop = false;
		this->cmbSettingsBaud->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsBaud_SelectedIndexChanged);
		// 
		// cmbSettingsSportCode
		// 
		this->cmbSettingsSportCode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsSportCode->Enabled = false;
		this->cmbSettingsSportCode->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsSportCode->FormattingEnabled = true;
		this->cmbSettingsSportCode->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L""});
		this->cmbSettingsSportCode->Location = System::Drawing::Point(179, 216);
		this->cmbSettingsSportCode->Name = L"cmbSettingsSportCode";
		this->cmbSettingsSportCode->Size = System::Drawing::Size(67, 23);
		this->cmbSettingsSportCode->TabIndex = 0;
		this->cmbSettingsSportCode->TabStop = false;
		// 
		// lblSettingsSerial
		// 
		this->lblSettingsSerial->AutoSize = true;
		this->lblSettingsSerial->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsSerial->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsSerial->Location = System::Drawing::Point(76, 164);
		this->lblSettingsSerial->Name = L"lblSettingsSerial";
		this->lblSettingsSerial->Size = System::Drawing::Size(49, 21);
		this->lblSettingsSerial->TabIndex = 0;
		this->lblSettingsSerial->Text = L"Serial";
		this->lblSettingsSerial->Click += gcnew System::EventHandler(this, &DataLinkForm::lblSettingsSerial_Click);
		// 
		// lblSettingsTitle
		// 
		this->lblSettingsTitle->AutoSize = true;
		this->lblSettingsTitle->BackColor = System::Drawing::Color::Transparent;
		this->lblSettingsTitle->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 14.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->lblSettingsTitle->Location = System::Drawing::Point(52, 11);
		this->lblSettingsTitle->Name = L"lblSettingsTitle";
		this->lblSettingsTitle->Size = System::Drawing::Size(154, 25);
		this->lblSettingsTitle->TabIndex = 0;
		this->lblSettingsTitle->Text = L"Stored Settings";
		// 
		// cmbSettingsPort
		// 
		this->cmbSettingsPort->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsPort->Enabled = false;
		this->cmbSettingsPort->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsPort->FormattingEnabled = true;
		this->cmbSettingsPort->Items->AddRange(gcnew cli::array< System::Object^  >(256) {L"", L"1", L"2", L"3", L"4", L"5", L"6", 
			L"7", L"8", L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16", L"17", L"18", L"19", L"20", L"21", L"22", L"23", L"24", L"25", 
			L"26", L"27", L"28", L"29", L"30", L"31", L"32", L"33", L"34", L"35", L"36", L"37", L"38", L"39", L"40", L"41", L"42", L"43", 
			L"44", L"45", L"46", L"47", L"48", L"49", L"50", L"51", L"52", L"53", L"54", L"55", L"56", L"57", L"58", L"59", L"60", L"61", 
			L"62", L"63", L"64", L"65", L"66", L"67", L"68", L"69", L"70", L"71", L"72", L"73", L"74", L"75", L"76", L"77", L"78", L"79", 
			L"80", L"81", L"82", L"83", L"84", L"85", L"86", L"87", L"88", L"89", L"90", L"91", L"92", L"93", L"94", L"95", L"96", L"97", 
			L"98", L"99", L"100", L"101", L"102", L"103", L"104", L"105", L"106", L"107", L"108", L"109", L"110", L"111", L"112", L"113", 
			L"114", L"115", L"116", L"117", L"118", L"119", L"120", L"121", L"122", L"123", L"124", L"125", L"126", L"127", L"128", L"129", 
			L"130", L"131", L"132", L"133", L"134", L"135", L"136", L"137", L"138", L"139", L"140", L"141", L"142", L"143", L"144", L"145", 
			L"146", L"147", L"148", L"149", L"150", L"151", L"152", L"153", L"154", L"155", L"156", L"157", L"158", L"159", L"160", L"161", 
			L"162", L"163", L"164", L"165", L"166", L"167", L"168", L"169", L"170", L"171", L"172", L"173", L"174", L"175", L"176", L"177", 
			L"178", L"179", L"180", L"181", L"182", L"183", L"184", L"185", L"186", L"187", L"188", L"189", L"190", L"191", L"192", L"193", 
			L"194", L"195", L"196", L"197", L"198", L"199", L"200", L"201", L"202", L"203", L"204", L"205", L"206", L"207", L"208", L"209", 
			L"210", L"211", L"212", L"213", L"214", L"215", L"216", L"217", L"218", L"219", L"220", L"221", L"222", L"223", L"224", L"225", 
			L"226", L"227", L"228", L"229", L"230", L"231", L"232", L"233", L"234", L"235", L"236", L"237", L"238", L"239", L"240", L"241", 
			L"242", L"243", L"244", L"245", L"246", L"247", L"248", L"249", L"250", L"251", L"252", L"253", L"254", L"255"});
		this->cmbSettingsPort->Location = System::Drawing::Point(56, 244);
		this->cmbSettingsPort->MaxDropDownItems = 5;
		this->cmbSettingsPort->Name = L"cmbSettingsPort";
		this->cmbSettingsPort->RightToLeft = System::Windows::Forms::RightToLeft::No;
		this->cmbSettingsPort->Size = System::Drawing::Size(64, 23);
		this->cmbSettingsPort->TabIndex = 0;
		this->cmbSettingsPort->TabStop = false;
		this->cmbSettingsPort->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsPort_SelectedIndexChanged);
		// 
		// btnDelete
		// 
		this->btnDelete->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDelete.BackgroundImage")));
		this->btnDelete->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnDelete->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnDelete->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnDelete->Location = System::Drawing::Point(186, 45);
		this->btnDelete->Name = L"btnDelete";
		this->btnDelete->Size = System::Drawing::Size(61, 23);
		this->btnDelete->TabIndex = 0;
		this->btnDelete->TabStop = false;
		this->btnDelete->Text = L"Delete";
		this->btnDelete->UseVisualStyleBackColor = true;
		this->btnDelete->Click += gcnew System::EventHandler(this, &DataLinkForm::btnDelete_Click);
		// 
		// btnNew
		// 
		this->btnNew->BackColor = System::Drawing::Color::Transparent;
		this->btnNew->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnNew.BackgroundImage")));
		this->btnNew->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnNew->FlatAppearance->BorderColor = System::Drawing::Color::White;
		this->btnNew->FlatAppearance->BorderSize = 0;
		this->btnNew->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnNew->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnNew->Location = System::Drawing::Point(56, 45);
		this->btnNew->Name = L"btnNew";
		this->btnNew->Size = System::Drawing::Size(61, 23);
		this->btnNew->TabIndex = 0;
		this->btnNew->TabStop = false;
		this->btnNew->Text = L"New";
		this->btnNew->UseVisualStyleBackColor = false;
		this->btnNew->Click += gcnew System::EventHandler(this, &DataLinkForm::btnNew_Click);
		// 
		// btnSettingsDatabase
		// 
		this->btnSettingsDatabase->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSettingsDatabase.BackgroundImage")));
		this->btnSettingsDatabase->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnSettingsDatabase->Enabled = false;
		this->btnSettingsDatabase->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnSettingsDatabase->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnSettingsDatabase->Location = System::Drawing::Point(78, 136);
		this->btnSettingsDatabase->Name = L"btnSettingsDatabase";
		this->btnSettingsDatabase->Size = System::Drawing::Size(73, 23);
		this->btnSettingsDatabase->TabIndex = 0;
		this->btnSettingsDatabase->TabStop = false;
		this->btnSettingsDatabase->Text = L"Database";
		this->btnSettingsDatabase->UseVisualStyleBackColor = true;
		this->btnSettingsDatabase->Click += gcnew System::EventHandler(this, &DataLinkForm::btnSettingsDatabase_Click);
		// 
		// cmbSettingsSport
		// 
		this->cmbSettingsSport->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
		this->cmbSettingsSport->Enabled = false;
		this->cmbSettingsSport->Font = (gcnew System::Drawing::Font(L"Courier New", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->cmbSettingsSport->FormattingEnabled = true;
		this->cmbSettingsSport->Items->AddRange(gcnew cli::array< System::Object^  >(1) {L""});
		this->cmbSettingsSport->Location = System::Drawing::Point(56, 216);
		this->cmbSettingsSport->MaxDropDownItems = 5;
		this->cmbSettingsSport->Name = L"cmbSettingsSport";
		this->cmbSettingsSport->Size = System::Drawing::Size(116, 23);
		this->cmbSettingsSport->TabIndex = 0;
		this->cmbSettingsSport->TabStop = false;
		this->cmbSettingsSport->SelectedIndexChanged += gcnew System::EventHandler(this, &DataLinkForm::cmbSettingsSport_SelectedIndexChanged);
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
		this->panel4->TabIndex = 99;
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
		this->linkNewTek->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &DataLinkForm::linkNewTek_LinkClicked);
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
		// btnExit
		// 
		this->btnExit->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnExit.BackgroundImage")));
		this->btnExit->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnExit->DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->btnExit->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnExit->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnExit->Location = System::Drawing::Point(450, 455);
		this->btnExit->Name = L"btnExit";
		this->btnExit->Size = System::Drawing::Size(83, 23);
		this->btnExit->TabIndex = 92;
		this->btnExit->Text = L"Exit";
		this->btnExit->UseVisualStyleBackColor = true;
		this->btnExit->Click += gcnew System::EventHandler(this, &DataLinkForm::btnExit_Click);
		// 
		// btnClearActive
		// 
		this->btnClearActive->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnClearActive.BackgroundImage")));
		this->btnClearActive->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnClearActive->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnClearActive->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnClearActive->Location = System::Drawing::Point(362, 455);
		this->btnClearActive->Name = L"btnClearActive";
		this->btnClearActive->Size = System::Drawing::Size(83, 23);
		this->btnClearActive->TabIndex = 91;
		this->btnClearActive->Text = L"Clear Settings";
		this->btnClearActive->UseVisualStyleBackColor = true;
		this->btnClearActive->Click += gcnew System::EventHandler(this, &DataLinkForm::btnClearActive_Click);
		// 
		// btnActivate
		// 
		this->btnActivate->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnActivate.BackgroundImage")));
		this->btnActivate->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
		this->btnActivate->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
		this->btnActivate->Font = (gcnew System::Drawing::Font(L"Segoe UI Symbol", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->btnActivate->Location = System::Drawing::Point(274, 455);
		this->btnActivate->Name = L"btnActivate";
		this->btnActivate->Size = System::Drawing::Size(83, 23);
		this->btnActivate->TabIndex = 90;
		this->btnActivate->Text = L">>Activate";
		this->btnActivate->UseVisualStyleBackColor = true;
		this->btnActivate->Click += gcnew System::EventHandler(this, &DataLinkForm::btnActivate_Click);
		// 
		// fadeInTimer
		// 
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkForm::fadeInTimer_Tick);
		// 
		// fadeOutTimer
		// 
		this->fadeOutTimer->Tick += gcnew System::EventHandler(this, &DataLinkForm::fadeOutTimer_Tick);
		// 
		// DataLinkForm
		// 
		this->AcceptButton = this->btnActivate;
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->BackColor = System::Drawing::Color::Silver;
		this->CancelButton = this->btnExit;
		this->ClientSize = System::Drawing::Size(541, 488);
		this->Controls->Add(this->panel1);
		this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
		this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
		this->MaximizeBox = false;
		this->MaximumSize = System::Drawing::Size(547, 512);
		this->MinimizeBox = false;
		this->MinimumSize = System::Drawing::Size(547, 512);
		this->Name = L"DataLinkForm";
		this->Opacity = 0;
		this->Text = L"DataLink Scoreboard Configuration";
		this->Load += gcnew System::EventHandler(this, &DataLinkForm::DataLinkForm_Load);
		this->panel1->ResumeLayout(false);
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->EndInit();
		this->panel3->ResumeLayout(false);
		this->panel3->PerformLayout();
		this->panel2->ResumeLayout(false);
		this->panel2->PerformLayout();
		this->panel4->ResumeLayout(false);
		this->panel4->PerformLayout();
		this->ResumeLayout(false);

	}
#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////
 //							GUI Elements							//
/////////////////////////////////////////////////////////////////////

	// Form load. Populate the form.
	System::Void DataLinkForm_Load(System::Object^  sender, System::EventArgs^  e)
	{
	}

#pragma region Buttons
	// Button: Exit program.
	System::Void btnExit_Click(System::Object^  sender, System::EventArgs^  e)
	{
		MyReg.RegWriteBool(cwsz_ActivePath,cwsz_UseTextFile,0);
		DataLinkUI::DataLinkForm::~DataLinkForm();
		Close();
	}

	// Button: Add new settings entry. Clears fields.
	System::Void btnNew_Click(System::Object^  sender, System::EventArgs^  e)
	{
		MyReg.RegWrite(	cwsz_ActivePath,
						REG_SZ,
						L"CurSettingName",
						L"");
		ClearAllSettings();
		MarkSaved();
	}

	// Button: Save settings entry.
	System::Void btnSave_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Grab values from GUI and save to Registry.
		string2wchar(this->cmbSettingsName->Text);
		size_t len = wcslen(wch);
		
		if(len)
		{
			size_t i = 0;
			wchar_t * tempString = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(wch) + 1)));
			wcscpy(tempString, (wchar_t *)wch);

			// Replace backslashes with spaces before saving.
			while (i < len)
			{
				if (tempString[i] == 0x5c)
					tempString[i] = 0x2d;
				i++;
			}

			// Write setting to registry.
			MyReg.RegWrite(	cwsz_ActivePath, REG_SZ, L"CurSettingName", wch);

			// Compose the path.
			const wchar_t * slash = L"\\";
			const size_t pathLen = wcslen(cwsz_SettingsPath); 
			wchar_t * tempPath = (wchar_t*)(_alloca(sizeof(wchar_t) * (pathLen + wcslen(tempString) + wcslen(slash) + 1))); 
			wcscpy(tempPath, cwsz_SettingsPath);
			wcscat(tempPath, slash);
			wcscat(tempPath, tempString);

			// Set up array to hold values for Reg write.
			const wchar_t * regValValue[eSettingsNumValues+1];

			regValValue[0]=tempString;
			{ string2wchar(this->cmbSettingsBoard->Text);		regValValue[1]=wch; }
			{ string2wchar(this->cmbSettingsSport->Text);		regValValue[2]=wch; }
			{ string2wchar(this->cmbSettingsSportCode->Text);	regValValue[3]=wch; }
			{ string2wchar(this->cmbSettingsPort->Text);		regValValue[4]=wch; }
			{ string2wchar(this->cmbSettingsBaud->Text);		regValValue[5]=wch; }
			{ string2wchar(this->cmbSettingsParity->Text);		regValValue[6]=wch; }
			{ string2wchar(this->cmbSettingsStopBits->Text);	regValValue[7]=wch; }
			{ string2wchar(this->cmbSettingsDataBits->Text);	regValValue[8]=wch; }

			// Write values to Settings key in Registry.
			for (int i=0; i<eSettingsNumValues; i++)
			{
				bool itWorked=MyReg.RegWrite(tempPath, REG_SZ, RegSettingsList[i].RegSettingsValName, regValValue[i]);

				if (itWorked)
					MarkSaved();
				else
					assert(false);
			}

			// Write the bool check boxes.
			MyReg.RegWriteBool(tempPath,cwsz_UseSerial,(this->chkSettingsSerial->Checked));
			MyReg.RegWriteBool(tempPath,cwsz_UseNetwork,(this->chkSettingsNetwork->Checked));
			MyReg.RegWriteBool(tempPath,cwsz_UseDatabase,(this->chkSettingsDatabase->Checked));

			// Add names to the drop-down combo box.
			PopulateSettingsNames();

			// Make only the appropriate buttons enabled in the settings side of the GUI.
			this->chkSettingsSerial->Checked ? EnableSettingsSerialValues() : DisableSettingsDatabaseBtn();
			this->cmbSettingsBoard->Text=="Generic" ? this->cmbSettingsSport->Enabled=false : this->cmbSettingsSport->Enabled=true;
			this->chkSettingsNetwork->Checked ? EnableSettingsNetworkBtn() : DisableSettingsNetworkBtn();
			this->chkSettingsDatabase->Checked ? EnableSettingsDatabaseBtn() : DisableSettingsDatabaseBtn();
		}
	}

	// Button: delete an entry from the list / registry.
	System::Void btnDelete_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Delete key for current entry name and reload dropdown list.
		string2wchar(this->cmbSettingsName->Text);
		MyReg.RegWrite(	cwsz_ActivePath,
						REG_SZ,
						L"CurSettingName",
						L"");
		MyReg.RegDelete(cwsz_SettingsPath, wch);
		ClearAllSettings();
		PopulateSettingsNames();
		MarkSaved();
	}

	// Button: Activates the currently selected stored setting entry. Copies values to active entry in registry.
	System::Void btnActivate_Click(System::Object^  sender, System::EventArgs^  e)
	{
		if (ValidateForBtnActivate())
			PopulateForBtnActivate();
//		else
//			assert(false);
	}

	// Clears the active settings.
	System::Void btnClearActive_Click(System::Object^  sender, System::EventArgs^  e)
	{
//		MyReg.RegDeleteATree(cwsz_RegPath, L"Active");	// Vista and Win7 only.
		MyReg.RegDeleteSubs(cwsz_ActiveQueryPath);
		MyReg.RegDeleteSubs(cwsz_ActiveSitesPath);
		MyReg.RegDeleteSubs(cwsz_ActivePath);
		MyReg.RegDelete(cwsz_RegPath, L"Active");
		ClearAllSettings();
		MarkSaved();
		PopulateActiveForLoad();
	}

	// Button: database button for modifying the database info / queries for a stored setting.
	// The database information and query data are writable.
	System::Void btnSettingsDatabase_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Set global to allow edit of entries.
		g_EnableDatabaseEntries = true;

		// Start database form.
		DataLinkDBForm^ dbDialog = gcnew DataLinkDBForm;

		this->Opacity=0.0f;

		// Disable parent form and activate database form. Reactivate once database form is closed.
		 this->Enabled=false;
		  this->Visible=false;

			dbDialog->ShowDialog(this);

		  this->Enabled=true;
		 this->Visible=true;

		 FadeIn();
	}

	// Button: Network RSS feed parsing info.
	System::Void btnSettingsNetwork_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Set global to allow edit of entries.
		g_EnableNetworkEntries = true;

		// Start network form.
		DataLinkNetForm^ nwDialog = gcnew DataLinkNetForm;

		this->Opacity=0.0f;

		// Disable parent form and activate database form. Reactivate once database form is closed.
		 this->Enabled=false;
		  this->Visible=false;

			nwDialog->ShowDialog(this);

		  this->Visible=true;
		 this->Enabled=true;
		 
		 FadeIn();
	}

	// Button: Active text settings. Simply opens the appropriate LiveText folder in Windows Explorer. User then edits text files.
	// ** If LiveText is not installed, will open Windows Explorer to user's default location (based upon system settings and OS) **
	System::Void btnActiveText_Click(System::Object^  sender, System::EventArgs^  e)
	{
#ifdef _DEBUG
		char exe_path[MAX_PATH];

		if (GetModuleFileNameA(NULL, exe_path, MAX_PATH))
		{
			char *lastSlash = strrchr(exe_path, '\\');
			if (lastSlash++)
				*lastSlash = 0;
			strcat(exe_path, "Text Input");
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
			const wchar_t * tempFolder = L"\\Stats Plugins\\Text Input";
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

	// Button: Database button for viewing the database info / queries for the currently active setting.
	// The database information and query data are write-protected.
	System::Void btnActiveDatabase_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Set global to dis-allow edit of entries.
		g_EnableDatabaseEntries = false;

		// Start the database form.
		DataLinkDBForm^ dbDialog = gcnew DataLinkDBForm;

		this->Opacity=0.0f;

		// Disable parent form and activate database form. Reactivate once database form is closed.
		 this->Enabled=false;
		  this->Visible=false;

			dbDialog->ShowDialog(this);

		  this->Visible=true;
		 this->Enabled=true;

		 FadeIn();
	}

	// Button: Future implementation of Network RSS feed parsing info.
	// ** For now opens the appropriate LiveText folder (if it exists) in Windows Explorer **
	// ** Currently disabled and not supported directly **
	System::Void btnActiveNetwork_Click(System::Object^  sender, System::EventArgs^  e)
	{
		// Set global to allow edit of entries.
		g_EnableNetworkEntries = false;

		// Start network form.
		DataLinkNetForm^ nwDialog = gcnew DataLinkNetForm;

		this->Opacity=0.0f;

		// Disable parent form and activate database form. Reactivate once database form is closed.
		 this->Enabled=false;
		  this->Visible=false;

			nwDialog->ShowDialog(this);

		  this->Enabled=true;
		 this->Visible=true;

		 FadeIn();
	}


	// Added a button to launch LiveText for convenience. Finds the install path from the registry.
	System::Void btnActiveLiveText_Click(System::Object^  sender, System::EventArgs^  e)
	{
		const wchar_t * tempVal = 0;

		tempVal = MyReg.RegRead2(cwsz_LiveTextPath, NULL, REG_SZ, TEXT("Directory"), tempVal);

		// If LiveText is installed and found, start it up.
		if (wcslen(tempVal) > 0)
		{
			const wchar_t * tempExec = L"\\LiveText.exe";
			const size_t pathLen = wcslen(tempVal);
			wchar_t * tempPath = (wchar_t*)(_alloca(sizeof(wchar_t) * (pathLen + wcslen(tempExec) + 1)));
			wcscpy(tempPath, tempVal);
			wcscat(tempPath, tempExec);
			wchar2string(tempPath);

			Process::Start(ss);
		}
	}

#pragma endregion

#pragma region Check and combo boxes
	// Settings check boxes enable/disable buttons.
	System::Void chkSettingsSerial_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->chkSettingsSerial->Checked)
			EnableSettingsSerialValues();
		else
		{
			DisableSettingsSerialValues();
			ClearPortSettings();
			this->cmbSettingsBoard->ResetText();
			this->cmbSettingsSport->ResetText();
			this->cmbSettingsSportCode->ResetText();
		}
		MarkUnsaved();
	}
	System::Void chkSettingsNetwork_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		this->chkSettingsNetwork->Checked ? EnableSettingsNetworkBtn() : DisableSettingsNetworkBtn();
		MarkUnsaved();
	}
	System::Void chkSettingsDatabase_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		this->chkSettingsDatabase->Checked ? EnableSettingsDatabaseBtn() : DisableSettingsDatabaseBtn();
		MarkUnsaved();
	}

	// Mark the save button as needing to be saved if drop-down is changed for several entries...
	System::Void cmbSettingsPort_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)		{MarkUnsaved();}
	System::Void cmbSettingsBaud_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)		{MarkUnsaved();}
	System::Void cmbSettingsDataBits_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)	{MarkUnsaved();}
	System::Void cmbSettingsParity_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)		{MarkUnsaved();}
	System::Void cmbSettingsStopBits_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)	{MarkUnsaved();}
	
	System::Void cmbSettingsName_TextChanged(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->cmbSettingsName->Text == "")
			MarkSaved();
		else
			MarkUnsaved();

	}

	// Link to the site.
	System::Void linkNewTek_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e)
	{
		// Link to the site.
		ShellExecute(NULL, L"open", L"http://www.newtek.com", NULL, NULL, SW_SHOWNORMAL);
	}

	// ** DEBUG ONLY **
	// Switch for testing only. Enables text input for test console that replaces serial input with info from file.
	// Text file is boardname.txt i.e. Daktronics.txt and they are to be placed in the LiveText\Stats Plugins folder.
	// Uses registry to track in order to enable multiple components to be in-sync.
	System::Void chkUseText_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
#ifdef _DEBUG
		if (this->chkUseText->Checked)
		{
			// Enable text input.
			MyReg.RegWriteBool(cwsz_ActivePath,cwsz_UseTextFile,1);
		}
		else
		{
			// Disable text input.
			MyReg.RegWriteBool(cwsz_ActivePath,cwsz_UseTextFile,0);
		}
#endif
	}

	// Handles the change of drop-down settings by reading registry values and changing fields accordingly.
	System::Void cmbSettingsName_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		// Update the values when a saved entry is selected from dropdown.
		const wchar_t * tempVal=0;
		System::String^ selectedName=this->cmbSettingsName->SelectedItem->ToString();
		string2wchar(selectedName);
		
		// Write the current active setting name to the registry.
		MyReg.RegWrite(	cwsz_ActivePath, REG_SZ, L"CurSettingName", wch);

		// Update GUI fields to registry values.
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("Board"), tempVal);
			wchar2string(tempVal); this->cmbSettingsBoard->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("DataBits"), tempVal);
			wchar2string(tempVal); this->cmbSettingsDataBits->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("Parity"), tempVal);
			wchar2string(tempVal); this->cmbSettingsParity->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("Port"), tempVal);
			wchar2string(tempVal); this->cmbSettingsPort->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("Sport"), tempVal);
			wchar2string(tempVal); this->cmbSettingsSport->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("SportCode"), tempVal);
			wchar2string(tempVal); this->cmbSettingsSportCode->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("StopBits"), tempVal);
			wchar2string(tempVal); this->cmbSettingsStopBits->Text=ss; }
		{ tempVal=MyReg.RegRead(cwsz_SettingsPath, wch, REG_SZ, TEXT("Baud"), tempVal);
			wchar2string(tempVal); this->cmbSettingsBaud->Text=ss; }

		wchar_t * tempPath=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_SettingsPath)+wcslen(wch)+3)));
		wcscpy(tempPath,cwsz_SettingsPath);
		wcscat(tempPath,L"\\");
		wcscat(tempPath,wch);

		this->chkSettingsSerial->Checked = MyReg.RegReadBool(tempPath,cwsz_UseSerial);
		this->chkSettingsNetwork->Checked = MyReg.RegReadBool(tempPath,cwsz_UseNetwork);
		this->chkSettingsDatabase->Checked = MyReg.RegReadBool(tempPath,cwsz_UseDatabase);

		// Make only the appropriate buttons enabled in the settings side of the GUI.
		this->chkSettingsSerial->Checked ? EnableSettingsSerialValues() : DisableSettingsDatabaseBtn();
		this->cmbSettingsBoard->Text=="Generic" ? this->cmbSettingsSport->Enabled=false : this->cmbSettingsSport->Enabled=true;
		this->chkSettingsNetwork->Checked ? EnableSettingsNetworkBtn() : DisableSettingsNetworkBtn();
		this->chkSettingsDatabase->Checked ? EnableSettingsDatabaseBtn() : DisableSettingsDatabaseBtn();

		MarkSaved();
}

	// Handles the change to and populates the board type drop-down.
	System::Void cmbSettingsBoard_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
		{
			// First, mark unsaved and clear the sport, sport code, and port setting drop downs.
			MarkUnsaved();
			this->cmbSettingsSport->Items->Clear();
			this->cmbSettingsSportCode->Items->Clear();
			ClearPortSettings();

			// Populate the sport (and sport code) list based upon the type of board.
			switch(this->cmbSettingsBoard->SelectedIndex)
			{
				case 0:	// Do nothing - first entry blank to allow for user clearing.
					break;
				case 1: //Generic
					this->cmbSettingsSport->Enabled=false;
					this->cmbSettingsBaud->SelectedIndex = 12;
					this->cmbSettingsDataBits->SelectedIndex = 4;
					this->cmbSettingsParity->SelectedIndex = 1;
					this->cmbSettingsStopBits->SelectedIndex = 2;
					break;
				case 2: //Daktronics
					this->cmbSettingsSport->Enabled=true;
					this->cmbSettingsSport->Items->Add("Baseball");
					this->cmbSettingsSport->Items->Add("Basketball");
					this->cmbSettingsSport->Items->Add("Football");
					this->cmbSettingsSport->Items->Add("Hockey");
					this->cmbSettingsSport->Items->Add("Soccer");
					this->cmbSettingsSport->Items->Add("Volleyball");
					break;
				case 3: //DSI
					this->cmbSettingsSport->Enabled=true;
					this->cmbSettingsSport->Items->Add("Basketball");
					break;
				case 4: //OES
					this->cmbSettingsSport->Enabled=true;
					this->cmbSettingsSport->Items->Add("Basketball");
					break;
				case 5: //TransLux
					this->cmbSettingsSport->Enabled=true;
					this->cmbSettingsSport->Items->Add("Basketball");
					this->cmbSettingsSport->Items->Add("Football");
					break;
				case 6: //WhiteWay
					this->cmbSettingsSport->Enabled=true;
					this->cmbSettingsSport->Items->Add("Basketball");
					break;
				default:
 					assert(false);
					break;
			}
		}

	System::Void cmbSettingsSport_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
	{
		MarkUnsaved();
		if (this->chkSettingsSerial->Checked)
		{
			// If sport is changed and board type is Daktronics...
			// (this is due to funky port differences between sports on a Daktronics 5000)
			if (this->cmbSettingsBoard->SelectedIndex == 1)
			{
				// Generic
			}
			else if (this->cmbSettingsBoard->SelectedIndex == 2 && this->cmbSettingsSport->SelectedIndex != 1)
			{
				// Daktronics are all 19,200 baud except Football code 6604 which is 9,600.
				this->cmbSettingsBaud->SelectedIndex = 9;
				this->cmbSettingsDataBits->SelectedIndex = 4;
				this->cmbSettingsParity->SelectedIndex = 1;
				this->cmbSettingsStopBits->SelectedIndex = 2;
			}
			// All other scoreboards (and Daktronics Football - code 6604) use 9,600 baud.
			else
			{
				this->cmbSettingsBaud->SelectedIndex = 7;
				this->cmbSettingsDataBits->SelectedIndex = 4;
				this->cmbSettingsParity->SelectedIndex = 1;
				this->cmbSettingsStopBits->SelectedIndex = 2;
			}
		}

		// Generic
		if (this->cmbSettingsBoard->Text=="Generic")
		{

		}
		// Daktronics
		else if (this->cmbSettingsBoard->Text=="Daktronics")
		{
			if (this->cmbSettingsSport->Text=="Basketball")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("1101");
			}
			if (this->cmbSettingsSport->Text=="Baseball")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("5501");
			}
			if (this->cmbSettingsSport->Text=="Football")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("6601");
			}
			if (this->cmbSettingsSport->Text=="Hockey")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("4401");
			}
			if (this->cmbSettingsSport->Text=="Soccer")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("7701");
			}
			if (this->cmbSettingsSport->Text=="Volleyball")
			{
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("2101");
			}
			// Select the first (currently -only- entry in sport code drop-down)
			this->cmbSettingsSportCode->SelectedIndex=0;
		}
		// DSI
		else if (this->cmbSettingsBoard->Text=="DSI")
		{
			if (this->cmbSettingsSport->Text=="Basketball")
			{
				// Set to Daktronics Basketball sport code list.
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("0");
			}
			// Select the first (currently -only- entry in sport code drop-down)
			this->cmbSettingsSportCode->SelectedIndex=0;
		}
		// OES
		else if (this->cmbSettingsBoard->Text=="OES")
		{
			if (this->cmbSettingsSport->Text=="Basketball")
			{
				// Set to Daktronics Basketball sport code list.
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("0");
			}
			// Select the first (currently -only- entry in sport code drop-down)
			this->cmbSettingsSportCode->SelectedIndex=0;
		}
		// TransLux
		else if (this->cmbSettingsBoard->Text=="TransLux")
		{
			if (this->cmbSettingsSport->Text=="Basketball" || this->cmbSettingsSport->Text=="Football")
			{
				// Set to Daktronics Basketball sport code list.
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("0");
			}
			// Select the first (currently -only- entry in sport code drop-down)
			this->cmbSettingsSportCode->SelectedIndex=0;
		}
		// WhiteWay
		else if (this->cmbSettingsBoard->Text=="WhiteWay")
		{
			if (this->cmbSettingsSport->Text=="Basketball")
			{
				// Set to Daktronics Basketball sport code list.
				this->cmbSettingsSportCode->Items->Clear();
				this->cmbSettingsSportCode->Items->Add("0");
			}
			// Select the first (currently -only- entry in sport code drop-down)
			this->cmbSettingsSportCode->SelectedIndex=0;
		}

	}
	System::Void lblSettingsSerial_Click(System::Object^  sender, System::EventArgs^  e)
	{
		this->chkSettingsSerial->Checked=(this->chkSettingsSerial->Checked ? false : true);
	}
#pragma endregion
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Utility Code

	// Clears all settings fields.
	void ClearAllSettings(void)
	{
		this->cmbSettingsName->ResetText();			
		this->cmbSettingsBoard->ResetText();
		this->chkSettingsSerial->Checked = false;
		this->chkSettingsNetwork->Checked = false;
		this->chkSettingsDatabase->Checked = false;
		this->cmbSettingsSport->ResetText();
		this->cmbSettingsSportCode->ResetText();
		this->cmbSettingsPort->ResetText();
		this->cmbSettingsBaud->ResetText();
		this->cmbSettingsParity->ResetText();
		this->cmbSettingsStopBits->ResetText();
		this->cmbSettingsDataBits->ResetText();
	}

	// Clears just port setting fields.
	void ClearPortSettings(void)
	{
		this->cmbSettingsPort->ResetText();
		this->cmbSettingsBaud->ResetText();
		this->cmbSettingsParity->ResetText();
		this->cmbSettingsStopBits->ResetText();
		this->cmbSettingsDataBits->ResetText();
	}

	// Marks save button to indicate changes need to be saved.
	void MarkUnsaved(void)
	{
		this->btnSave->Text="Save*";
		this->btnSave->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif",
														   8.25F,
														   System::Drawing::FontStyle::Bold,
														   System::Drawing::GraphicsUnit::Point, 
														   static_cast<System::Byte>(0)));
		// Disable database and network buttons when an entry has not been saved. This avoids
		//   trying to define and save detailed database/network settings to an undefined set of settings.
		DisableSettingsDatabaseBtn();
		DisableSettingsNetworkBtn();
	}
	
	// Marks save button as saved or unchanged.
	void MarkSaved(void)
	{
		this->btnSave->Text="Save";
		this->btnSave->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif",
														   8.25F,
														   System::Drawing::FontStyle::Regular,
														   System::Drawing::GraphicsUnit::Point, 
														   static_cast<System::Byte>(0)));
	}

	// Validates the settings fields based upon connection type in order to then populate active settings values.
	bool ValidateForBtnActivate(void)
	{
		bool ret = true;
		
		if (!(this->chkSettingsNetwork->Checked)	&&	!(this->chkSettingsDatabase->Checked)	&&
			!(this->chkSettingsSerial->Checked))
			ret = false;

		// If any required serial value is empty or no types are checked, then fail the validate.
		else if	(this->chkSettingsSerial->Checked)
		{
			if ((this->cmbSettingsPort->Text == "")		|| (this->cmbSettingsBaud->Text == "")		||
				(this->cmbSettingsParity->Text == "")	|| (this->cmbSettingsStopBits->Text == "")	||
				(this->cmbSettingsDataBits->Text == "")	|| (this->cmbSettingsBoard->Text == "")		||
				((this->cmbSettingsSport->Text == "") && (this->cmbSettingsBoard->Text != "Generic")))
				ret = false;
		}

		return ret;
	}

	// Populates the settings for the active device.
	void PopulateForBtnActivate(void)
	{
		const wchar_t * valname=L"";

		{ valname=L"Name"; string2wchar(this->cmbSettingsName->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveName->Text=this->cmbSettingsName->Text; }
		{ valname=L"Board"; string2wchar(this->cmbSettingsBoard->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveBoard->Text=this->cmbSettingsBoard->Text; }
		{ valname=L"Sport"; string2wchar(this->cmbSettingsSport->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveSport->Text=this->cmbSettingsSport->Text; }
		{ valname=L"SportCode"; string2wchar(this->cmbSettingsSportCode->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveSportCode->Text=this->cmbSettingsSportCode->Text; }
		{ valname=L"Port"; string2wchar(this->cmbSettingsPort->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActivePort->Text=this->cmbSettingsPort->Text; }
		{ valname=L"Baud"; string2wchar(this->cmbSettingsBaud->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveBaud->Text=this->cmbSettingsBaud->Text; }
		{ valname=L"Parity"; string2wchar(this->cmbSettingsParity->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveParity->Text=this->cmbSettingsParity->Text; }
		{ valname=L"StopBits"; string2wchar(this->cmbSettingsStopBits->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveStopBits->Text=this->cmbSettingsStopBits->Text; }
		{ valname=L"DataBits"; string2wchar(this->cmbSettingsDataBits->Text); MyReg.RegWrite(cwsz_ActivePath, REG_SZ, valname, wch);
			this->tboxActiveDataBits->Text=this->cmbSettingsDataBits->Text; }

		this->chkActiveSerial->Checked = this->chkSettingsSerial->Checked;
		this->chkActiveNetwork->Checked = this->chkSettingsNetwork->Checked;
		this->chkActiveDatabase->Checked = this->chkSettingsDatabase->Checked;

		MyReg.RegWriteBool(cwsz_ActivePath, cwsz_UseSerial, (this->chkActiveSerial->Checked));
		MyReg.RegWriteBool(cwsz_ActivePath, cwsz_UseNetwork, (this->chkActiveNetwork->Checked));
		MyReg.RegWriteBool(cwsz_ActivePath, cwsz_UseDatabase, (this->chkActiveDatabase->Checked));

		this->chkActiveNetwork->Checked ? EnableActiveNetworkBtn() : DisableActiveNetworkBtn();
		this->chkActiveDatabase->Checked ? EnableActiveDatabaseBtn() : DisableActiveDatabaseBtn();

		// Copy database info and queries from setting to active in registry...
		CopyDBInfoToActive();
		CopyNWInfoToActive();
	}

	// Grabs the database connection info / queries from the currently selected (saved) setting and copies to active.
	void CopyDBInfoToActive(void)
	{
		const wchar_t * CurSettingName = 0;
		wchar_t * pathSettingsToUse = 0;
		wchar_t * pathSettingsQueryToUse = 0;
		wchar_t * pathActiveToUse = 0;
		const wchar_t * tempVal = 0;
		const wchar_t * tempCopy = 0;
		size_t NumEntries = 0;

		// Generate the active and settings paths needed.
		CurSettingName=MyReg.RegRead(cwsz_ActivePath,NULL,REG_SZ,L"CurSettingName",CurSettingName);
		pathSettingsToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_SettingsPath)+wcslen(CurSettingName)+3)));
		wcscpy(pathSettingsToUse, cwsz_SettingsPath);
		wcscat(pathSettingsToUse, L"\\");
		wcscat(pathSettingsToUse, CurSettingName);
		pathActiveToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_ActivePath)+wcslen(CurSettingName)+8)));
		wcscpy(pathActiveToUse, cwsz_ActivePath);
		wcscat(pathActiveToUse, L"\\Query");

		// Write database to active settings in registry.
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"Driver",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"Driver",tempCopy);
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"Server",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"Server",tempCopy);
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"Database",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"Database",tempCopy);
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"UserID",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"UserID",tempCopy);
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"Password",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"Password",tempCopy);
		tempCopy=MyReg.RegRead(pathSettingsToUse,NULL,REG_SZ,L"Timeout",tempCopy);
		MyReg.RegWrite(cwsz_ActivePath,REG_SZ,L"Timeout",tempCopy);

		// Deletes the active query folder in the registry and the old values it may have contained.
		MyReg.RegDeleteSubs(pathActiveToUse);

		pathSettingsQueryToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathSettingsToUse)+wcslen(L"\\Query\\")+256)));
		wcscpy(pathSettingsQueryToUse, pathSettingsToUse);
		wcscat(pathSettingsQueryToUse, L"\\Query\\");

		// Enumerate and write each SQL key/query to the registry in a loop.
		HKEY hKey;
		DWORD retCode;

		retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
								pathSettingsQueryToUse,
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

			// 		TCHAR achValue[MAX_VALUE_NAME]; 
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
						// Get the paths and write the SQL query key, command, and 'has multiple keys' flag.
						wchar_t * pathActiveQuery=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathActiveToUse)+wcslen(achKey) + 3)));
						wcscpy(pathActiveQuery,pathActiveToUse);
						wcscat(pathActiveQuery,L"\\");
						wcscat(pathActiveQuery,achKey);
						wchar_t * pathSettingsQuery=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathSettingsQueryToUse)+wcslen(achKey) + 3)));
						wcscpy(pathSettingsQuery,pathSettingsQueryToUse);
						wcscat(pathSettingsQuery,L"\\");
						wcscat(pathSettingsQuery,achKey);

						const wchar_t * command = 0;
						const wchar_t * key = 0;

						command=MyReg.RegRead(pathSettingsQueryToUse,achKey,REG_SZ,L"Command",command);
						MyReg.RegWrite(pathActiveQuery,REG_SZ,L"Command",command);

						MyReg.RegWriteBool(pathActiveQuery,L"Multi",(MyReg.RegReadBool(pathSettingsQuery,L"Multi")));
					}
				}
			}
		}
	}

	void CopyNWInfoToActive(void)
	{
		const wchar_t * CurSettingName = 0;
		wchar_t * pathSettingsToUse = 0;
		wchar_t * pathSettingsSitesToUse = 0;
		wchar_t * pathActiveToUse = 0;
		const wchar_t * tempVal = 0;
		const wchar_t * tempCopy = 0;
		size_t NumEntries = 0;

		// Generate the active and settings paths needed.
		CurSettingName=MyReg.RegRead(cwsz_ActivePath,NULL,REG_SZ,L"CurSettingName",CurSettingName);
		pathSettingsToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_SettingsPath)+wcslen(CurSettingName)+3)));
		wcscpy(pathSettingsToUse, cwsz_SettingsPath);
		wcscat(pathSettingsToUse, L"\\");
		wcscat(pathSettingsToUse, CurSettingName);
		pathActiveToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_ActivePath)+wcslen(CurSettingName)+8)));
		wcscpy(pathActiveToUse, cwsz_ActivePath);
		wcscat(pathActiveToUse, L"\\Sites");

		// Write network to active settings in registry.

		// Deletes the active sites folder in the registry and the old values it may have contained.
		MyReg.RegDeleteSubs(pathActiveToUse);

		pathSettingsSitesToUse=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathSettingsToUse)+wcslen(L"\\Sites\\")+256)));
		wcscpy(pathSettingsSitesToUse, pathSettingsToUse);
		wcscat(pathSettingsSitesToUse, L"\\Sites");

		// Enumerate and write each site to the registry in a loop.
		HKEY hKey;
		DWORD retCode;

		retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
								pathSettingsSitesToUse,
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

			// 		TCHAR achValue[MAX_VALUE_NAME]; 
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
						// Get the paths and write site info.
						wchar_t * pathActiveSites=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathActiveToUse)+wcslen(achKey) + 3)));
						wcscpy(pathActiveSites,pathActiveToUse);
						wcscat(pathActiveSites,L"\\");
						wcscat(pathActiveSites,achKey);
						wchar_t * pathSettingsSites=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(pathSettingsSitesToUse)+wcslen(achKey) + 3)));
						wcscpy(pathSettingsSites,pathSettingsSitesToUse);
						wcscat(pathSettingsSites,L"\\");
						wcscat(pathSettingsSites,achKey);

						const wchar_t * address = 0;

						// Write site info to registry.
						MyReg.RegWrite(pathActiveSites,REG_SZ,L"Address",(MyReg.RegRead(pathSettingsSites,NULL,REG_SZ,L"Address",address)));
						MyReg.RegWriteNum(pathActiveSites, L"Blocksize", (MyReg.RegReadNum(pathSettingsSites, L"Blocksize")));
						MyReg.RegWriteNum(pathActiveSites, L"Refresh", (MyReg.RegReadNum(pathSettingsSites, L"Refresh")));
						MyReg.RegWriteNum(pathActiveSites, L"Timeout", (MyReg.RegReadNum(pathSettingsSites, L"Timeout")));
						MyReg.RegWriteBool(pathActiveSites,L"Enabled",(MyReg.RegReadBool(pathSettingsSites,L"Enabled")));
					}
				}
			}
		}
	}

	// Loads the activated values and populates the fields - used upon form load.
	void PopulateActiveForLoad(void)
	{
		const wchar_t * val=0;
		
		// Set activated values in GUI to data stored in registry.
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[0].RegActiveValName, val); wchar2string(val); this->tboxActiveName->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[1].RegActiveValName, val); wchar2string(val); this->tboxActiveBoard->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[2].RegActiveValName, val); wchar2string(val); this->tboxActiveSport->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[3].RegActiveValName, val); wchar2string(val); this->tboxActiveSportCode->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[4].RegActiveValName, val); wchar2string(val); this->tboxActivePort->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[5].RegActiveValName, val); wchar2string(val); this->tboxActiveBaud->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[6].RegActiveValName, val); wchar2string(val); this->tboxActiveParity->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[7].RegActiveValName, val); wchar2string(val); this->tboxActiveStopBits->Text=ss; }
		{ val=MyReg.RegRead(cwsz_ActivePath, NULL, REG_SZ, RegActiveList[8].RegActiveValName, val); wchar2string(val); this->tboxActiveDataBits->Text=ss; }	

		this->chkActiveSerial->Checked = MyReg.RegReadBool(cwsz_ActivePath, cwsz_UseSerial);
		this->chkActiveNetwork->Checked = MyReg.RegReadBool(cwsz_ActivePath, cwsz_UseNetwork);
		this->chkActiveDatabase->Checked = MyReg.RegReadBool(cwsz_ActivePath, cwsz_UseDatabase);

		this->chkActiveNetwork->Checked ? EnableActiveNetworkBtn() : DisableActiveNetworkBtn();
		this->chkActiveDatabase->Checked ? EnableActiveDatabaseBtn() : DisableActiveDatabaseBtn();
	}

	// Loads the settings entries.
	void PopulateSettingsNames(void)
	{
		// Clear the dropdown.
		this->cmbSettingsName->Items->Clear();

		// Create dropdown list for names of saved settings.
		const wchar_t * val=0;

		// Enumerates and reads settings entries from the registry and writes to GUI entries.
		HKEY hKey;
		DWORD retCode;

		retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
								cwsz_SettingsPath,
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
				for (unsigned i=0; i<cSubKeys; i++) 
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
						// Update GUI fields.
						wchar2string(achKey);
						this->cmbSettingsName->Items->Add(ss);
					}
				}
			}
		}
// 		else
//  			assert(false);
	}

	// Show settings serial fields.
	void EnableSettingsSerialValues()
	{
		this->cmbSettingsBoard->Enabled=true;
		this->cmbSettingsSport->Enabled=true;

		this->cmbSettingsBaud->Enabled=true;
		this->cmbSettingsDataBits->Enabled=true;
		this->cmbSettingsParity->Enabled=true;
		this->cmbSettingsPort->Enabled=true;
		this->cmbSettingsStopBits->Enabled=true;
	}

	// Hide settings serial fields.
	void DisableSettingsSerialValues()
	{
		this->cmbSettingsBoard->Enabled=false;
		this->cmbSettingsSport->Enabled=false;

		this->cmbSettingsBaud->Enabled=false;
		this->cmbSettingsDataBits->Enabled=false;
		this->cmbSettingsParity->Enabled=false;
		this->cmbSettingsPort->Enabled=false;
		this->cmbSettingsStopBits->Enabled=false;
	}

	// Functions to enable buttons based upon context.
	void EnableActiveNetworkBtn()	{this->btnActiveNetwork->Enabled=true;}
	void DisableActiveNetworkBtn()	{this->btnActiveNetwork->Enabled=false;}
	void EnableActiveDatabaseBtn()	{this->btnActiveDatabase->Enabled=true;}
	void DisableActiveDatabaseBtn() {this->btnActiveDatabase->Enabled=false;}
	void EnableActiveTextBtn()		{this->btnActiveText->Enabled=true;}
	void DisableActiveTextBtn()		{this->btnActiveText->Enabled=false;}

	void EnableSettingsNetworkBtn()		{this->btnSettingsNetwork->Enabled=true;}
	void DisableSettingsNetworkBtn()	{this->btnSettingsNetwork->Enabled=false;}
	void EnableSettingsDatabaseBtn()	{this->btnSettingsDatabase->Enabled=true;}
	void DisableSettingsDatabaseBtn()	{this->btnSettingsDatabase->Enabled=false;}
#pragma endregion

private:
	System::Void fadeInTimer_Tick(System::Object^  sender, System::EventArgs^  e)
	{
		if (this->Opacity < 1.0f)
 			this->Opacity = (this->Opacity + .005f) * 1.1f;
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
 			this->Opacity = (this->Opacity - .005f) * 0.1f;
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
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkForm::fadeInTimer_Tick);
	}

	void FadeOut()
	{
		fadeCounter = 0;
		fadeInTimer->Interval = 10;
		fadeInTimer->Enabled = true;
		this->fadeInTimer->Tick += gcnew System::EventHandler(this, &DataLinkForm::fadeOutTimer_Tick);
	}
};

}

