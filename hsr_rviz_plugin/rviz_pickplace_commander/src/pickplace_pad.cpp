#include "pickplace_pad.h"

namespace rviz_pickplace_commander
{
    /* ���캯������ʼ������ */
    PickPlacePanel::PickPlacePanel(QWidget* parent) : rviz::Panel(parent),openSizeMax(1000),\
                                        openSizeMin(0),gripperSpeed(500),gripperForce(100)
    {	
        std::cout<<"111111111"<<std::endl;
		//QTextCodec::setCodecForTr(QTextCodec::codecForLocale());	/* ���������ַ� */
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312")); /* ���������ַ� */
	
	/******************������ݳ�ʼ��*********************************************************************/
		m_trainingMdoel = new TRAINED_MODEL[1];
   
    /******************�������*********************************************************************/
        m_serialWdg = new SerialWdg;
		m_gripperWdg = new GripperWdg;
		m_orkWdg = new OrkWdg;
		m_pickPlaceWdg = new PickPlaceWdg;
					    		
        /* ���岼�� */
        mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_serialWdg->serialTitleLabel,1);
        mainLayout->addWidget(m_serialWdg,1);
		mainLayout->addWidget(m_gripperWdg->gripperTitleLabel,1);
        mainLayout->addWidget(m_gripperWdg,3);
		mainLayout->addWidget(m_orkWdg->ORKtitleLabel,1);
        mainLayout->addWidget(m_orkWdg,4);
		mainLayout->addWidget(m_pickPlaceWdg->pickPlacetitleLabel,1);
        mainLayout->addWidget(m_pickPlaceWdg,7);
		
        setLayout(mainLayout);
		
    /******************�ۺ���*********************************************************************/
	    connect(m_serialWdg->serialNoComoBox,SIGNAL(clicked(bool)),this,SLOT(slotSerialNoComoBoxClicked()));
		connect(m_serialWdg->serialConnectOrNotBtn,SIGNAL(pressed()),this,SLOT(slotSerialConnectBtn()));
        connect(m_gripperWdg->gripperSetComoBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotGripperParmSetComboBox(int)));
        connect(m_gripperWdg->gripperSetBtn,SIGNAL(pressed()),this,SLOT(slotGripperSetBtn()));
		connect(m_gripperWdg->gripperActRunBtn,SIGNAL(clicked(bool)),this,SLOT(slotGripperActRunBtn()));
		
		connect(m_orkWdg->addOrDeleteComoBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotAddOrDeleteComboBox(int)));
		connect(m_orkWdg->toSelectDeleteComoBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotToSelectDeleteComoBox(int)));
		//connect(canRecognModelComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotCanRecognModelComboBox(int)));
		
		connect(m_orkWdg->newAddOrDeleteOkBtn,SIGNAL(pressed()),this,SLOT(slotNewAddOrDeleteOkBtn()));
		connect(m_orkWdg->modelPathOpenBtn,SIGNAL(pressed()),this,SLOT(slotModelPathOpenBtn()));
		connect(m_orkWdg->addMeshOkBtn,SIGNAL(pressed()),this,SLOT(slotAddMeshOkBtn()));
		connect(m_orkWdg->toTrainBtn,SIGNAL(clicked(bool)),this,SLOT(slotToTrainBtn()));
		connect(m_orkWdg->toRecognBtn,SIGNAL(pressed()),this,SLOT(slotToRecognBtn()));

		connect(m_orkWdg->recognizedPoseComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotToSelectDetectComoBox(int)));
		
		connect(m_pickPlaceWdg->getPickPoseMeans,SIGNAL(currentIndexChanged(int)),this,SLOT(slotGetPickPoseMeansComoBox(int)));
		connect(m_pickPlaceWdg->getPlacePoseMeans,SIGNAL(currentIndexChanged(int)),this,SLOT(slotGetPlacePoseMeansComoBox(int)));
		connect(m_pickPlaceWdg->getPlacePoseOkBtn,SIGNAL(pressed()),this,SLOT(slotGetPlacePoseOkBtn()));
		
		connect(m_pickPlaceWdg->runPickPlaceBtn,SIGNAL(clicked(bool)),this,SLOT(slotRunPickPlaceBtn()));
	/******************ROS���*********************************************************************/
        /* �������ڴ򿪿ͻ��� */
	    client_serialOpen = n_pickPlace.serviceClient<hsr_gripper_driver::serial_open_srv>("serial_open");
        client_gripperOpenSize = n_pickPlace.serviceClient<hsr_gripper_driver::open_size_srv>("gripper_set_open_size");
		client_gripperOpen = n_pickPlace.serviceClient<hsr_gripper_driver::open_srv>("gripper_open");
		client_gripperClose = n_pickPlace.serviceClient<hsr_gripper_driver::close_srv>("gripper_close");
		client_gripperStop = n_pickPlace.serviceClient<hsr_gripper_driver::stop_srv>("gripper_stop");
		
		/* ����ORKģ����ؿͻ��� */
		client_objAdd = n_pickPlace.serviceClient<ork_interface::objAdd>("object_add");
		client_objDelete = n_pickPlace.serviceClient<ork_interface::objDelete>("object_delete");
	    client_meshAdd = n_pickPlace.serviceClient<ork_interface::meshAdd>("mesh_add");
		client_objSearch = n_pickPlace.serviceClient<ork_interface::objSearch>("object_search");
		client_training = n_pickPlace.serviceClient<ork_interface::training_srv>("training");
		client_detection = n_pickPlace.serviceClient<ork_interface::detection_srv>("detection");
		
		/* ����ץȡ�����ģ����ؿͻ��� */
		client_pickPlace = n_runPickPlace.serviceClient<hsr_pick::pickPlace>("pick_and_place");
	/*******************��ʱ��**************************************************************/
	    getPickPoseFromORK_Timer = new QTimer(this);
	    connect(getPickPoseFromORK_Timer,SIGNAL(timeout()),this,SLOT(slotGetPickPoseFromOrk()));
		
		//showPoseFromCamera_Timer = new QTimer(this);
        //connect(showPoseFromCamera_Timer,SIGNAL(timeout()),this,SLOT(slotShowPoseFromCameraTimer()));

		
		//������ʼ��
		canRecognizedModelListInit();
        path_detect_config = "";
        num_detectedObj = 0;
	
    }
	
	PickPlacePanel::~PickPlacePanel()
	{
		//
		delete mainLayout;
		delete m_pickPlaceWdg;
		delete m_orkWdg;
		delete m_gripperWdg;
		delete m_serialWdg;
		delete[] m_trainingMdoel;
		delete[] m_poseFromCamera;
	}
	
	void PickPlacePanel::slotSerialNoComoBoxClicked()
	{
		//
		m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("join the function of slotSerialNoComoBoxClicked"));
		
		glob_t ttyUSBpath_buf;
        int i;
        glob("/dev/ttyUSB*",GLOB_NOSORT, NULL, &ttyUSBpath_buf);		
        
		m_serialWdg->serialNoComoBox->clear();           /* �����òۺ���������豸��ѡ�ţ���ֹ�ھɵ��豸�ź�׷�� */
		
        for(i=0; i < ttyUSBpath_buf.gl_pathc; i++)
        {
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::fromStdString(ttyUSBpath_buf.gl_pathv[i]));
			
			QFileInfo ttyUSBpath_Info = QFileInfo(QString::fromStdString(ttyUSBpath_buf.gl_pathv[i]));			
			/* ����������ttyUSB���ӵ���ѡ�豸���������� */
			m_serialWdg->serialNoComoBox->addItem(ttyUSBpath_Info.fileName()); 
        }
 
        globfree(&ttyUSBpath_buf);
	}
	
	void PickPlacePanel::slotSerialConnectBtn()
	{
		//`rospack find object_recognition_linemod`/conf/detection.ros.ork
        //const char *cmd = "rospack find object_recognition_linemod";
        //char result[1024]={0};
       
        //executeCMD(cmd,result);
		//printf("%s", result);
		
		hsr_gripper_driver::serial_open_srv serialOpen_srv;

		/* ��ǰѡ�еĴ����豸�źͲ�����ת��string���ͺ� ����������*/
		serialOpen_srv.request.serialNo = (std::string)("/dev/")+ m_serialWdg->serialNoComoBox->currentText().toStdString();
		serialOpen_srv.request.baudrate = m_serialWdg->baudrateComoBox->currentText().toInt();
        m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Baudrate:");
		m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::number(serialOpen_srv.request.baudrate, 10));
		m_pickPlaceWdg->pickPlaceStatusTextEdit->append("SerialNo:");
		m_pickPlaceWdg->pickPlaceStatusTextEdit->append(m_serialWdg->serialNoComoBox->currentText());
		
		if(client_serialOpen.call(serialOpen_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Serial Port opened!!!");
		}
		else
		{
		    m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Unable to open port!!!");
		}	
	}
		
	void PickPlacePanel::canRecognizedModelListInit()
	{
		//**********************************����������¿�ʶ��ģ���б�*******************************************
				
		ork_interface::objSearch objSearch_srv;
		
		if(m_orkWdg->toSelectDeleteComoBox->count())
		{
			//�������ѡ���ǰ����ִ��disconnect����������ڵ�ǰ�����ĸı䣬����currentIndexChanged���Ӷ����±���
			disconnect(m_orkWdg->toSelectDeleteComoBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDeleteComoBox(int)));
			m_orkWdg->toSelectDeleteComoBox->clear();
			connect(m_orkWdg->toSelectDeleteComoBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDeleteComoBox(int)));
			
			m_orkWdg->canRecognModelComboBox->clear();
		}
			
		if(client_objSearch.call(objSearch_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("get the Name of the object sucessful!!!");

		    //pickPlaceStatusTextEdit->append(QString::fromStdString(objSearch_srv.response.objName[1]));
					
			// ��ȡ��ѵ���������
			int num_trained = objSearch_srv.response.objNum;
					
			if(!num_trained)
				return;
			delete[] m_trainingMdoel;
			m_trainingMdoel = new TRAINED_MODEL[num_trained];
						
			for(int i = 0;i <num_trained;i++)
			{
				m_trainingMdoel[i].name = objSearch_srv.response.objName[i];
				m_trainingMdoel[i].id = objSearch_srv.response.objId[i];
				m_trainingMdoel[i].description = objSearch_srv.response.objDescription[i];
				
				m_orkWdg->canRecognModelComboBox->addItem((QString)("Name:") + QString::fromStdString(m_trainingMdoel[i].name)+ \
				                                (QString)(";ID:") + QString::fromStdString(m_trainingMdoel[i].id));
                m_orkWdg->toSelectDeleteComoBox->addItem(QString::fromStdString(m_trainingMdoel[i].name));												
			}
			
			int index_current = m_orkWdg->toSelectDeleteComoBox->currentIndex();
			m_orkWdg->newAddModelDescripLineEdit->setText(QString::fromStdString(m_trainingMdoel[index_current].description));
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Get the detectable model failed,please check whether object_search is running!!!");
		}
		
		if(0 == m_orkWdg->addOrDeleteComoBox->currentIndex())
		{
			m_orkWdg->newAddModelDescripLineEdit->setText("");
		}
	}

	void PickPlacePanel::slotGripperParmSetComboBox(int index)
	{
        switch(index)
        {
            case 0:  //���ü�צ����
            {
                m_gripperWdg->openMaxLabel->setText("��󿪿�:");
                m_gripperWdg->openMinLabel->setText("��С����:");

				m_gripperWdg->openMinLabel->setVisible(true);
                m_gripperWdg->openMinLineEdit->setVisible(true);
				
                m_gripperWdg->openMaxLineEdit->setValidator(new QIntValidator(0, 1000, this));
				m_gripperWdg->openMinLineEdit->setValidator(new QIntValidator(0, 1000, this));
				m_gripperWdg->openMaxLineEdit->setPlaceholderText(tr("����0-1000������"));
		        m_gripperWdg->openMaxLineEdit->setToolTip(tr("����0-1000����������ֵԽ�󿪿�Խ��\r\n�Ҹ�ֵ��Ҫ���ڡ�������Сֵ��"));
				m_gripperWdg->openMaxLineEdit->setText(QString::number(openSizeMax));
				m_gripperWdg->openMinLineEdit->setText(QString::number(openSizeMin));
				
			    break;
            }
			case 1:  //���ü�צ�ٶ�
            {
				m_gripperWdg->openMaxLabel->setText("��С:");
				m_gripperWdg->openMaxLineEdit->setValidator(new QIntValidator(1, 1000, this));
				m_gripperWdg->openMaxLineEdit->setPlaceholderText("����0-1000������");
		        m_gripperWdg->openMaxLineEdit->setToolTip("����1-1000����������ֵԽ���ٶ�Խ��");
				m_gripperWdg->openMaxLineEdit->setText(QString::number(gripperSpeed));

				m_gripperWdg->openMinLabel->setVisible(false);
                m_gripperWdg->openMinLineEdit->setVisible(false);
				
				break;
			}
			case 2:  //���ü�צ����
			{
				m_gripperWdg->openMaxLabel->setText("��С:");
				m_gripperWdg->openMaxLineEdit->setValidator(new QIntValidator(50, 1000, this));
				m_gripperWdg->openMaxLineEdit->setPlaceholderText("����50-1000������");
		        m_gripperWdg->openMaxLineEdit->setToolTip("����50-1000����������ֵԽ������Խ��");
				m_gripperWdg->openMaxLineEdit->setText(QString::number(gripperForce));

				m_gripperWdg->openMinLabel->setVisible(false);
                m_gripperWdg->openMinLineEdit->setVisible(false);
				
				break;
			}
			default:
				break;
        }
	}
	void PickPlacePanel::slotGripperSetBtn()
	{
	    int index = m_gripperWdg->gripperSetComoBox->currentIndex();
	    switch(index)
	    {
	    	case 0:  //���ü�צ����
	    	{
                hsr_gripper_driver::open_size_srv gripperOpenSize_srv;

		        gripperOpenSize_srv.request.max = m_gripperWdg->openMaxLineEdit->text().toInt();
		        gripperOpenSize_srv.request.min = m_gripperWdg->openMinLineEdit->text().toInt();

				if(client_gripperOpenSize.call(gripperOpenSize_srv))
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Set the open size of the Gripper sucessful!!!"));
					openSizeMax = m_gripperWdg->openMaxLineEdit->text().toInt();
					openSizeMin = m_gripperWdg->openMinLineEdit->text().toInt();
				}
				else
				{
				    m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Set the open size of the Gripper failed!!!"));
				}	
				
			    break;
	    	}
			case 1:  //���ü�צ�ٶ�
				gripperSpeed = m_gripperWdg->openMaxLineEdit->text().toInt();
				
				break;
			case 2:  //���ü�צ����
                gripperForce = m_gripperWdg->openMaxLineEdit->text().toInt();
				
				break;
			default:
				break;
	    }
	}
	void PickPlacePanel::slotGripperActRunBtn()
	{
	    //
	    switch(m_gripperWdg->gripperActComoBox->currentIndex())
	    {
	    	case 0:     //��צ��
	    	{
                hsr_gripper_driver::open_srv gripperOpen_srv;
		        gripperOpen_srv.request.speed = gripperSpeed;
				if(client_gripperOpen.call(gripperOpen_srv))
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Open the Gripper sucessful!!!"));
				}
				else
				{
				    m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Open the Gripper failed!!!"));
				}	
				
				break;
	    	}
			case 1:    //��צ�ر�
			{
                hsr_gripper_driver::close_srv gripperClose_srv;
				
		        gripperClose_srv.request.speed = gripperSpeed;
				gripperClose_srv.request.force = gripperForce;
				
				if(client_gripperClose.call(gripperClose_srv))
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Close the Gripper sucessful!!!"));

				}
				else
				{
				    m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Close the Gripper failed!!!"));
				}	
				break;
			}
			case 2:    //��צ��ͣ
			{
				hsr_gripper_driver::stop_srv gripperStop_srv;
				
				if(client_gripperStop.call(gripperStop_srv))
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Stop the Gripper sucessful!!!"));

				}
				else
				{
				    m_pickPlaceWdg->pickPlaceStatusTextEdit->append(tr("Stop the Gripper failed!!!"));
				}	

				break;
			}
			default:

				break;
	    }
	}
	
	void PickPlacePanel::slotAddOrDeleteComboBox(int index)
	{
		//
		switch(index)
	    {
	    	case 0:     //�л��� ����ģ��
	    	{	
				m_orkWdg->newAddOrDeleteOkBtn->setText("ȷ������");
				
				//m_orkWdg->toSelectDeleteComoBox->removeItem(0);
				
				m_orkWdg->newAddModelNameLineEdit->setVisible(true);
				m_orkWdg->toSelectDeleteComoBox->setVisible(false);
				
				m_orkWdg->newAddModelNameLineEdit->setText("");
				m_orkWdg->newAddModelDescripLineEdit->setText("");
				
				int count_toSelectDeleteComoBox = m_orkWdg->toSelectDeleteComoBox->count();
				
				m_pickPlaceWdg->pickPlaceStatusTextEdit->append("count_toSelectDeleteComoBox:");
				m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::number(count_toSelectDeleteComoBox, 10));
				
				break;
	    	}
			case 1:    //�л��� ɾ��ģ��
			{
				m_orkWdg->newAddOrDeleteOkBtn->setText("ȷ��ɾ��");
				
				m_orkWdg->newAddModelNameLineEdit->setVisible(false);
				m_orkWdg->toSelectDeleteComoBox->setVisible(true);
				
				m_orkWdg->newAddModelDescripLineEdit->setText("");
				
				int index = m_orkWdg->toSelectDeleteComoBox->currentIndex();
				/* ��toSelectDeleteComoBoxΪ��ʱ��index = -1*/
				if(index >= 0)
				{
		            m_orkWdg->newAddModelDescripLineEdit->setText(QString::fromStdString(m_trainingMdoel[index].description));
				}
				
				break;			
			}
			default:
				break;
	    }
		
	}
	
	void PickPlacePanel::slotToSelectDeleteComoBox(int index)
	{
		m_orkWdg->newAddModelDescripLineEdit->setText("");
		m_orkWdg->newAddModelDescripLineEdit->setText(QString::fromStdString(m_trainingMdoel[index].description));
	}
	
	void PickPlacePanel::slotNewAddOrDeleteOkBtn()
	{
		//
		switch(m_orkWdg->addOrDeleteComoBox->currentIndex())
		{
			case 0:  //ȷ������
			{
				ork_interface::objAdd objAdd_srv;
				objAdd_srv.request.name = m_orkWdg->newAddModelNameLineEdit->text().toStdString();
				objAdd_srv.request.description = m_orkWdg->newAddModelDescripLineEdit->text().toStdString();
						
				if(client_objAdd.call(objAdd_srv))
				{
					newAddModel_ID = QString::fromStdString(objAdd_srv.response.id);
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("new add a object sucessful!!!");
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("*************************************");
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("the new add object ID is��");
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::fromStdString(objAdd_srv.response.id));
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("*************************************");
					
					canRecognizedModelListInit();
				}
				else
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("new add a object failed!!!");
				}
				
				break;
			}
			case 1:  //ȷ��ɾ��
			{
                ork_interface::objDelete objDelete_srv;
				
				int index_delete = m_orkWdg->toSelectDeleteComoBox->currentIndex();
				
		        objDelete_srv.request.id = m_trainingMdoel[index_delete].id;
				m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Delete a object ID:");
				m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::fromStdString(objDelete_srv.request.id));
				
				if(client_objDelete.call(objDelete_srv))
				{
					m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Delete a object sucessful!!!");
					
					//ɾ��ĳ������ѡ���ǰ����ִ��disconnect����������ڵ�ǰ�����ĸı䣬����currentIndexChanged���Ӷ����±���
				    disconnect(m_orkWdg->toSelectDeleteComoBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDeleteComoBox(int)));
				    m_orkWdg->toSelectDeleteComoBox->removeItem(index_delete);
				    connect(m_orkWdg->toSelectDeleteComoBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDeleteComoBox(int)));
					
					canRecognizedModelListInit();
				}
				else
				{
				    m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Delete a object failed!!!");
				}
						
				break;
			}
			default:
				break;
		}	
	}

	void PickPlacePanel::slotModelPathOpenBtn()
	{
	    //
	    QString modelFilePath=QFileDialog::getOpenFileName(this,"open modelFile dialog","/","Model files(*.stl)");

        m_orkWdg->modelPathLineEdit->setText(modelFilePath);

		m_pickPlaceWdg->pickPlaceStatusTextEdit->append("The Model Path is:");
        m_pickPlaceWdg->pickPlaceStatusTextEdit->append(modelFilePath);
				
	}
	
	void PickPlacePanel::slotAddMeshOkBtn()
	{
		//
		ork_interface::meshAdd meshAdd_srv;
		meshAdd_srv.request.id = newAddModel_ID.toStdString();
		meshAdd_srv.request.path = m_orkWdg->modelPathLineEdit->text().toStdString();
			
		if(client_meshAdd.call(meshAdd_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("new add a mesh sucessful!!!");
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("new add a mesh failed!!!");
		}
	}
	
	void PickPlacePanel::executeCMD(const char *cmd, char *result)
	{
		//
		char buf_ps[1024];
        char ps[1024]={0};
        FILE *ptr;
        strcpy(ps, cmd);
        if((ptr=popen(ps, "r"))!=NULL)
        {
            while(fgets(buf_ps, 1024, ptr)!=NULL)
            {
                // ����ͨ����������ȡshell�������е�ÿһ�е����
                // printf("%s", buf_ps);
                strcat(result, buf_ps);
                if(strlen(result)>1024)
                break;
            }
        pclose(ptr);
        ptr = NULL;
        }
        else
        {
            printf("popen %s error\n", ps);
        }
	}
	
	void PickPlacePanel::slotToTrainBtn()
	{
		//
        ork_interface::training_srv training_srv;
		
		//�ȼ���ִ�� rosrun object_recognition_core training -c /home/eima/Work/catkin_ws/src/linemod/conf/training.ork
		const char *cmd = "rospack find object_recognition_linemod";
        char path_result[1024]={0};
	
        executeCMD(cmd,path_result);              /* ִ����������� */
		path_result[strlen(path_result)-1] = 0;   /* ɾ����ĩβ��\n */
		std::string s_path_result = path_result;  /* ��char[]תΪstd::string */
		
		training_srv.request.cmd = "\'c\'"; 
		training_srv.request.path = s_path_result + "/conf/training.ork";
			
		if(client_training.call(training_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Training sucessful!!!");
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Training failed!!!");
		}		

	}

	void PickPlacePanel::slotToRecognBtn()
	{	
		ork_interface::detection_srv detection_srv;
		
		//�ȼ���ִ�� rosrun object_recognition_core detection -c  `rospack find object_recognition_linemod`/conf/detection.ros.ork
		const char *cmd = "rospack find object_recognition_linemod";
        char path_result[1024]={0};
	
        executeCMD(cmd,path_result);              /* ִ����������� */
		path_result[strlen(path_result)-1] = 0;   /* ɾ����ĩβ��\n */
		std::string s_path_result = path_result;  /* ��char[]תΪstd::string */
		
		detection_srv.request.cmd = "\'c\'"; 
		detection_srv.request.path = s_path_result + "/conf/detection.ros.ork";
        path_detect_config = detection_srv.request.path;
        m_pickPlaceWdg->pickPlaceStatusTextEdit->append("******num_detectedObj****");
        m_pickPlaceWdg->pickPlaceStatusTextEdit->append(QString::number(num_detectedObj, 10));
        detection_srv.request.isDetected = false;
	
		if(client_detection.call(detection_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection start sucessful!!!");
			sub_poseFromCamera = n_pickPlace.subscribe("/recognized_object_array", 1000, &PickPlacePanel::poseFromCamera_callback, this);
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection start failed!!!");
		}
		
	}
	
	void PickPlacePanel::poseFromCamera_callback(const object_recognition_msgs::RecognizedObjectArray::ConstPtr& msg)
	{
		num_detectedObj = 0;
		num_detectedObj = msg->objects.size();
        ROS_ERROR("num_detectedObj:%d",num_detectedObj);
		
		if(!num_detectedObj)
			return;
       
        ork_interface::detection_srv detection_srv;
        detection_srv.request.cmd = "\'c\'"; 
		detection_srv.request.path = path_detect_config;
        detection_srv.request.isDetected = true;
        
        /* �����⵽���壬��ֹͣORK������ */
        if(client_detection.call(detection_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection stop sucessful!!!");
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection stop failed!!!");
		}
		
        /* �����µĶ�̬����ǰ����delete */
        //delete[] detectPoseFromCamera;
        //delete[] base_detectPoseFromCamera;
        //delete[] m_poseFromCamera;

		detectPoseFromCamera = new geometry_msgs::PoseStamped[num_detectedObj];
		base_detectPoseFromCamera = new geometry_msgs::PoseStamped[num_detectedObj];
		m_poseFromCamera = new T_POSE_FORM_CAMERA[num_detectedObj];
		
		tf::TransformListener listener;
		geometry_msgs::PoseStamped base_temp;
		
		for(int i = 0;i< num_detectedObj;i++)
		{
			detectPoseFromCamera[i].header.seq = 1;
			detectPoseFromCamera[i].header.frame_id = "kinect2_rgb_optical_frame";
			
			detectPoseFromCamera[i].pose = msg->objects[i].pose.pose.pose;
            ROS_ERROR("detectPoseFromCamera[i].pose.X:%f",detectPoseFromCamera[i].pose.position.x);
			
			 while(1)
            {
                static int count =0;
				try
				{
					listener.transformPose("base_link",detectPoseFromCamera[i],base_detectPoseFromCamera[i]);
				}
				catch(tf::TransformException &ex)
				{
		            ROS_ERROR("%s",ex.what());
		            ros::Duration(1.0).sleep();
		            continue;
				}

                if((base_detectPoseFromCamera[i].pose.position.x !=0.0 )&& (base_detectPoseFromCamera[i].pose.position.y !=0.0))
                     break;
                /* ѭ��100�Σ�����δ�������������� */
                count++;
                if(count >100)
                {
                     ROS_ERROR("form Camera frame to Base frame failed !!!");
                     break;
                }
             }
		}

       static int count_update =0;
       ROS_ERROR("**************************************");
       ROS_ERROR("the count_update is %d",count_update);
       ROS_ERROR("**************************************");
       
       if(num_detectedObj != pro_num_detectedObj )
             count_update =0; 

       //if(!count_update)
       //{
		
        disconnect(m_orkWdg->recognizedPoseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDetectComoBox(int)));
		m_orkWdg->recognizedPoseComboBox->clear();
        
        ROS_ERROR("**************************************");
        ROS_ERROR("the count of objects is %d",num_detectedObj);
        ROS_ERROR("**************************************");
		for(int i=0;i<num_detectedObj;i++)
		{
			m_orkWdg->recognizedPoseComboBox->addItem("ʶ��Ŀ��"+QString::number(i+1, 10));
			
			m_poseFromCamera[i].X = base_detectPoseFromCamera[i].pose.position.x;
		    m_poseFromCamera[i].Y = base_detectPoseFromCamera[i].pose.position.y;
		    m_poseFromCamera[i].Z = base_detectPoseFromCamera[i].pose.position.z;
		
		    float q0 = m_poseFromCamera[i].quaterW = base_detectPoseFromCamera[i].pose.orientation.w;
		    float q1 = m_poseFromCamera[i].quaterX = base_detectPoseFromCamera[i].pose.orientation.x;
		    float q2 = m_poseFromCamera[i].quaterY = base_detectPoseFromCamera[i].pose.orientation.y;
		    float q3 = m_poseFromCamera[i].quaterZ = base_detectPoseFromCamera[i].pose.orientation.z;
		
		    m_poseFromCamera[i].eulerR = atan2(2*(q0*q1+q2*q3),1-2*(q1*q1+q2*q2));
		    m_poseFromCamera[i].eulerP = asin(2*(q0*q2-q1*q3));
		    m_poseFromCamera[i].eulerY = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3));

            ROS_ERROR("ʶ��Ŀ��:%d,��X����Ϊ%f",i,m_poseFromCamera[i].X);
				
		}
        
        connect(m_orkWdg->recognizedPoseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToSelectDetectComoBox(int)));
        
        if((m_poseFromCamera[0].X == 0.0) && (m_poseFromCamera[0].Y == 0.0) && (m_poseFromCamera[0].Z==0.0))
     		return;

		m_orkWdg->recognizedPoseComboBox->setCurrentIndex(0);

		m_orkWdg->ORKposeXlineEdit->setText(QString::number(m_poseFromCamera[0].X, 'f', 3));   
		m_orkWdg->ORKposeYlineEdit->setText(QString::number(m_poseFromCamera[0].Y, 'f', 3));
		m_orkWdg->ORKposeZlineEdit->setText(QString::number(m_poseFromCamera[0].Z, 'f', 3));
		
		m_orkWdg->ORKquaterXlineEdit->setText(QString::number(m_poseFromCamera[0].quaterX, 'f', 3));
		m_orkWdg->ORKquaterYlineEdit->setText(QString::number(m_poseFromCamera[0].quaterY, 'f', 3));
		m_orkWdg->ORKquaterZlineEdit->setText(QString::number(m_poseFromCamera[0].quaterZ, 'f', 3));
		m_orkWdg->ORKquaterWlineEdit->setText(QString::number(m_poseFromCamera[0].quaterW, 'f', 3));
		
		m_orkWdg->ORKeulerRlineEdit->setText(QString::number(m_poseFromCamera[0].eulerR*180/PI, 'f', 3));
		m_orkWdg->ORKeulerPlineEdit->setText(QString::number(m_poseFromCamera[0].eulerP*180/PI, 'f', 3));
		m_orkWdg->ORKeulerYlineEdit->setText(QString::number(m_poseFromCamera[0].eulerY*180/PI, 'f', 3));
        ROS_ERROR("X:%f",m_poseFromCamera[0].X);
      //}
      count_update++;
      pro_num_detectedObj = num_detectedObj;
	}
	
	void PickPlacePanel::slotToSelectDetectComoBox(int index)
	{
		m_orkWdg->ORKposeXlineEdit->setText(QString::number(m_poseFromCamera[index].X, 'f', 3));    /* X*1000 ����λ��mת��mm */
		m_orkWdg->ORKposeYlineEdit->setText(QString::number(m_poseFromCamera[index].Y, 'f', 3));
		m_orkWdg->ORKposeZlineEdit->setText(QString::number(m_poseFromCamera[index].Z, 'f', 3));
		
		m_orkWdg->ORKquaterXlineEdit->setText(QString::number(m_poseFromCamera[index].quaterX, 'f', 3));
		m_orkWdg->ORKquaterYlineEdit->setText(QString::number(m_poseFromCamera[index].quaterY, 'f', 3));
		m_orkWdg->ORKquaterZlineEdit->setText(QString::number(m_poseFromCamera[index].quaterZ, 'f', 3));
		m_orkWdg->ORKquaterWlineEdit->setText(QString::number(m_poseFromCamera[index].quaterW, 'f', 3));
		
		m_orkWdg->ORKeulerRlineEdit->setText(QString::number(m_poseFromCamera[index].eulerR*180/PI, 'f', 3));
		m_orkWdg->ORKeulerPlineEdit->setText(QString::number(m_poseFromCamera[index].eulerP*180/PI, 'f', 3));
		m_orkWdg->ORKeulerYlineEdit->setText(QString::number(m_poseFromCamera[index].eulerY*180/PI, 'f', 3));
	}
	
	void PickPlacePanel::slotGetPickPoseFromOrk()
	{
		//
		m_pickPlaceWdg->pickXlineEdit->setText(m_orkWdg->ORKposeXlineEdit->text());
		m_pickPlaceWdg->pickYlineEdit->setText(m_orkWdg->ORKposeYlineEdit->text());
		m_pickPlaceWdg->pickZlineEdit->setText(m_orkWdg->ORKposeZlineEdit->text());
		m_pickPlaceWdg->pickEulerRlineEdit->setText(m_orkWdg->ORKeulerRlineEdit->text());
		m_pickPlaceWdg->pickEulerPlineEdit->setText(m_orkWdg->ORKeulerPlineEdit->text());
		m_pickPlaceWdg->pickEulerYlineEdit->setText(m_orkWdg->ORKeulerYlineEdit->text());
			
		m_pickPlaceWdg->pickQuaterXlineEdit->setText(m_orkWdg->ORKquaterXlineEdit->text());
		m_pickPlaceWdg->pickQuaterYlineEdit->setText(m_orkWdg->ORKquaterYlineEdit->text());
		m_pickPlaceWdg->pickQuaterZlineEdit->setText(m_orkWdg->ORKquaterZlineEdit->text());
		m_pickPlaceWdg->pickQuaterWlineEdit->setText(m_orkWdg->ORKquaterWlineEdit->text());
		
	}
	
    void PickPlacePanel::slotGetPlacePoseOkBtn()
	{
		//
		//sub_poseFrom3d = n_pickPlace.subscribe("/rviz_moveit_motion_planning_display/robot_interaction_interactive_marker_topic/feedback", 1000, &PickPlacePanel::poseFrom3d_callback, this);
    
        ork_interface::detection_srv detection_srv;
        detection_srv.request.cmd = "\'c\'"; 
		detection_srv.request.path = path_detect_config;
        detection_srv.request.isDetected = true;
        
        if(client_detection.call(detection_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection stop sucessful!!!");
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Detection stop failed!!!");
		}
    
	}
	void PickPlacePanel::poseFrom3d_callback(const visualization_msgs::InteractiveMarkerFeedback::ConstPtr& msg)
	{
		//
		float X = msg->pose.position.x;
		float Y = msg->pose.position.y;
		float Z = msg->pose.position.z;
		
		float q0 = msg->pose.orientation.w;
		float q1 = msg->pose.orientation.x;
		float q2 = msg->pose.orientation.y;
		float q3 = msg->pose.orientation.z;
		
		float A = atan2(2*(q0*q1+q2*q3),1-2*(q1*q1+q2*q2));
		float B = asin(2*(q0*q2-q1*q3));
		float C = atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3));
		
		if(m_pickPlaceWdg->getPlacePoseMeans->currentIndex()==0)
		{
		    m_pickPlaceWdg->placeXlineEdit->setText(QString::number(X, 'f', 3));    /* X*1000 ����λ��mת��mm */
		    m_pickPlaceWdg->placeYlineEdit->setText(QString::number(Y, 'f', 3));
		    m_pickPlaceWdg->placeZlineEdit->setText(QString::number(Z, 'f', 3));
		
		    m_pickPlaceWdg->placeQuaterXlineEdit->setText(QString::number(q1, 'f', 3));
		    m_pickPlaceWdg->placeQuaterYlineEdit->setText(QString::number(q2, 'f', 3));
		    m_pickPlaceWdg->placeQuaterZlineEdit->setText(QString::number(q3, 'f', 3));
		    m_pickPlaceWdg->placeQuaterWlineEdit->setText(QString::number(q0, 'f', 3));
		
		    m_pickPlaceWdg->placeEulerAlineEdit->setText(QString::number(A*180/PI, 'f', 3));
		    m_pickPlaceWdg->placeEulerBlineEdit->setText(QString::number(B*180/PI, 'f', 3));
		    m_pickPlaceWdg->placeEulerClineEdit->setText(QString::number(C*180/PI, 'f', 3));
		}
		
		if(m_pickPlaceWdg->getPickPoseMeans->currentIndex()==1)
		{
		    m_pickPlaceWdg->pickXlineEdit->setText(QString::number(X, 'f', 3));    /* X*1000 ����λ��mת��mm */
		    m_pickPlaceWdg->pickYlineEdit->setText(QString::number(Y, 'f', 3));
		    m_pickPlaceWdg->pickZlineEdit->setText(QString::number(Z, 'f', 3));
		
		    m_pickPlaceWdg->pickQuaterXlineEdit->setText(QString::number(q1, 'f', 3));
		    m_pickPlaceWdg->pickQuaterYlineEdit->setText(QString::number(q2, 'f', 3));
		    m_pickPlaceWdg->pickQuaterZlineEdit->setText(QString::number(q3, 'f', 3));
		    m_pickPlaceWdg->pickQuaterWlineEdit->setText(QString::number(q0, 'f', 3));
		
		    m_pickPlaceWdg->pickEulerRlineEdit->setText(QString::number(A*180/PI, 'f', 3));
		    m_pickPlaceWdg->pickEulerPlineEdit->setText(QString::number(B*180/PI, 'f', 3));
		    m_pickPlaceWdg->pickEulerYlineEdit->setText(QString::number(C*180/PI, 'f', 3));
		}
	}
	
	void PickPlacePanel::slotGetPickPoseMeansComoBox(int index)
	{
		switch(index)
		{
			case 0:  //�Զ�ʶ��
			{
                //��ʱ��δ����ʱ��������ʱ��
				if(getPickPoseFromORK_Timer->isActive() == false)
				{
				    getPickPoseFromORK_Timer->start(100);                      /* ������ʱ������ORKʶ��ģ���ȡץȡ��λ�� */
				}
				
				m_pickPlaceWdg->pickXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickYlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickEulerRlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickEulerPlineEdit->setReadOnly(true);
			    m_pickPlaceWdg->pickEulerYlineEdit->setReadOnly(true);
				
				m_pickPlaceWdg->pickQuaterXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterYlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterWlineEdit->setReadOnly(true);
				
				break;
			}
			case 1:  //�ռ��ȡ
			{
				getPickPoseFromORK_Timer->stop();                              /* �رն�ʱ��������ORKʶ��ģ���ȡץȡ��λ�� */
				
				sub_poseFrom3d = n_pickPlace.subscribe("/rviz_moveit_motion_planning_display/robot_interaction_interactive_marker_topic/feedback", 1000,\
                                                       &PickPlacePanel::poseFrom3d_callback, this);
				
				m_pickPlaceWdg->pickXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickYlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickEulerRlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickEulerPlineEdit->setReadOnly(true);
			    m_pickPlaceWdg->pickEulerYlineEdit->setReadOnly(true);
				
				m_pickPlaceWdg->pickQuaterXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterYlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->pickQuaterWlineEdit->setReadOnly(true);
				
				break;
			}
		    case 2:  //�ֶ�����
			{
				getPickPoseFromORK_Timer->stop();                              /* �رն�ʱ��������ORKʶ��ģ���ȡץȡ��λ�� */
				
				m_pickPlaceWdg->pickXlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickYlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickZlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickEulerRlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickEulerPlineEdit->setReadOnly(false);
			    m_pickPlaceWdg->pickEulerYlineEdit->setReadOnly(false);
				
				m_pickPlaceWdg->pickQuaterXlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickQuaterYlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickQuaterZlineEdit->setReadOnly(false);
				m_pickPlaceWdg->pickQuaterWlineEdit->setReadOnly(false);
				
			    break;
			}
		    default:
			    break;
		}
	}
	
	void PickPlacePanel::slotGetPlacePoseMeansComoBox(int index)
	{
		//
		switch(index)
		{
			case 0:  //�ռ��ȡ
			{
				sub_poseFrom3d = n_pickPlace.subscribe("/rviz_moveit_motion_planning_display/robot_interaction_interactive_marker_topic/feedback", 1000,\
                                                       &PickPlacePanel::poseFrom3d_callback, this);
													   
			    m_pickPlaceWdg->placeXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeYlineEdit->setReadOnly(true);
			    m_pickPlaceWdg->placeZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeEulerAlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeEulerBlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeEulerClineEdit->setReadOnly(true);
				
				m_pickPlaceWdg->placeQuaterXlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeQuaterYlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeQuaterZlineEdit->setReadOnly(true);
				m_pickPlaceWdg->placeQuaterWlineEdit->setReadOnly(true);
				
				break;
			}
		    case 1:  //�ֶ�����
			{
				m_pickPlaceWdg->placeXlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeYlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeZlineEdit->setReadOnly(false);
		
		        m_pickPlaceWdg->placeQuaterXlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeQuaterYlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeQuaterZlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeQuaterWlineEdit->setReadOnly(false);
		
		        m_pickPlaceWdg->placeEulerAlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeEulerBlineEdit->setReadOnly(false);
		        m_pickPlaceWdg->placeEulerClineEdit->setReadOnly(false);
			    break;
			}
		    default:
			    break;
		}
	}
	
	void PickPlacePanel::slotRunPickPlaceBtn()
	{
		//client_pickPlace		
	    m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Ready to pick...");	
		hsr_pick::pickPlace pickPlace_srv;
		
		geometry_msgs::PoseStamped pickPose,placePose;
	
		pickPose.pose.position.x = m_pickPlaceWdg->pickXlineEdit->text().toDouble();
		pickPose.pose.position.y = m_pickPlaceWdg->pickYlineEdit->text().toDouble();
		pickPose.pose.position.z = m_pickPlaceWdg->pickZlineEdit->text().toDouble();
		pickPose.pose.orientation.x = m_pickPlaceWdg->pickQuaterXlineEdit->text().toDouble();
		pickPose.pose.orientation.y = m_pickPlaceWdg->pickQuaterYlineEdit->text().toDouble();
		pickPose.pose.orientation.z = m_pickPlaceWdg->pickQuaterZlineEdit->text().toDouble();
		pickPose.pose.orientation.w = m_pickPlaceWdg->pickQuaterWlineEdit->text().toDouble();
		
		placePose.pose.position.x = m_pickPlaceWdg->placeXlineEdit->text().toDouble();
		placePose.pose.position.y = m_pickPlaceWdg->placeYlineEdit->text().toDouble();
		placePose.pose.position.z = m_pickPlaceWdg->placeZlineEdit->text().toDouble();
		placePose.pose.orientation.x = m_pickPlaceWdg->placeQuaterXlineEdit->text().toDouble();
		placePose.pose.orientation.y = m_pickPlaceWdg->placeQuaterYlineEdit->text().toDouble();
		placePose.pose.orientation.z = m_pickPlaceWdg->placeQuaterZlineEdit->text().toDouble();
		placePose.pose.orientation.w = m_pickPlaceWdg->placeQuaterWlineEdit->text().toDouble();	

		pickPlace_srv.request.pickPos = pickPose;
		pickPlace_srv.request.placePos = placePose;
		
		if(client_pickPlace.call(pickPlace_srv))
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Join the pickPlace_srv sucessful!!!");
			ROS_INFO("Join the pickPlace_srv sucessful!!!");
		}
		else
		{
			m_pickPlaceWdg->pickPlaceStatusTextEdit->append("Join the pickPlace_srv failed!!!");
			ROS_ERROR("Join the pickPlace_srv failed!!!");
		}		
	}
}

/* ����������һ��rviz�Ĳ�� */
#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_pickplace_commander::PickPlacePanel,rviz::Panel )

/* end control_pad.cpp*/
