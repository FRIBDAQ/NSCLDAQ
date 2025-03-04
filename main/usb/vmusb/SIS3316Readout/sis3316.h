/************************************************************************************************/
/*                                                                                              */
/*  Filename: sis3316.h                                                                         */
/*                                                                                              */
/*  Funktion: headerfile for SIS3316/SIS3316-2                                                  */
/*                                                                                              */
/*  Autor:                TH                                                                    */
/*  date:                 03.07.2012                                                            */
/*  last modification:    05.07.2021                                                            */
/*                                                                                              */
/*                                                                                              */
/*  modification:    10.12.2012																	*/
/*  add: #define SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG 	0xA4                                    */	  
/*                                                                                              */
/*  modification:    22.01.2013                                                                 */
/*  add: #define SIS3316_HARDWARE_VERSION				0x1C									*/
/*  add: #define SIS3316_KEY_USER_FUNCTION				0x404									*/
/*                                                                                              */
/*  modification:    10.04.2013                                                                 */
/*  add: #define SIS3316_KEY_ENABLE_SAMPLE_BANK_SWAP_CONTROL_WITH_NIM_INPUT		0x428			*/
/*                                                                                              */
/*  modification:    20.11.2013                                                                 */
/*  add: ENERGY defines																			*/
/*                                                                                              */
/*  modification:    10.12.2013                                                                 */
/*  add: AVERAGE_CONFIGURATION defines															*/
/*                                                                                              */
/*  modification:    31.01.2014                                                                 */
/*  changed:                              														*/
/*       old: define SIS3316_DATA_TRANSFER_CH12_16_CTRL_REG										*/
/*       new: define SIS3316_DATA_TRANSFER_CH13_16_CTRL_REG										*/
/*       old: define SIS3316_DATA_TRANSFER_ADC12_16_STATUS_REG									*/
/*       new: define SIS3316_DATA_TRANSFER_ADC13_16_STATUS_REG									*/
/*                                                                                              */
/*  modification:    20.08.2014                                                                 */
/*  add:                              														    */
/*     #define SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x1098	        */	  
/*     #define SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x2098	        */	  
/*     #define SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x3098	        */	  
/*     #define SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x4098	        */	  
/*                                                                                              */
/*  modification:    21.11.2014                                                                 */
/*  add:                              														    */
/*                                                                                              */
/*     #define SIS3316_KEY_PPS_LATCH_BIT_CLEAR                 				0x430		        */
/*                                                                                              */
/*     #define SIS3316_ADC_CH1_4_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x1130				*/	  
/*     #define SIS3316_ADC_CH1_4_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x1134		        */  
/*     #define SIS3316_ADC_CH5_8_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x2130		        */  
/*     #define SIS3316_ADC_CH5_8_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x2134		        */  
/*																								*/
/*     #define SIS3316_ADC_CH9_12_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x3130	        */
/*     #define SIS3316_ADC_CH9_12_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x3134	        */ 
/*     #define SIS3316_ADC_CH13_16_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x4130	        */ 
/*     #define SIS3316_ADC_CH13_16_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x4134	        */
/*                                                                                              */
/*                                                                                              */
/*  modification:    06.02.2015                                                                 */
/*  add:                              														    */
/*                                                                                              */
/*     #define SIS3316_LOOKUP_TABLE_CONTROL_REG                  			0x64		        */
/*     #define SIS3316_LOOKUP_TABLE_ADDR_REG                  				0x68		        */
/*     #define SIS3316_LOOKUP_TABLE_DATA_REG                  				0x6C		        */
/*                                                                                              */
/*  modification:    06.03.2015                                                                 */
/*  add:                              														    */
/*     #define SIS3316_PRESCALER_OUTPUT_PULSE_DIVIDER_REG  					0xB8                */
/*     #define SIS3316_PRESCALER_OUTPUT_PULSE_LENGTH_REG  					0xBC                */
/*     #define SIS3316_KEY_DISABLE_PRESCALER_OUTPUT_PULSE_DIVIDER_LOGIC	    0x42C               */
/*                                                                                              */
/*  modification:    25.06.2015                                                                 */
/*  add:                              														    */
/*     #define SIS3316_ADC_CH1_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x10E0	  	    */
/*     .......                                                                                  */
/*     #define SIS3316_ADC_CH16_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG		0x40EC	  	    */
/*                                                                                              */
/*                                                                                              */
/*  modification:    15.09.2016                                                                 */
/*  add:                              														    */
/*     #define SIS3316_INTERNAL_TRIGGER_FEEDBACK_SELECT_REG					0x7C                */
/*                                                                                              */
/*                                                                                              */
/*                                                                                              */
/*  modification:    19.10.2016                                                                 */
/*  change from:                              												    */
/*     #define SIS3316_ADC_CH9_PREVIOUS_BANK _SAMPLE_ADDRESS_REG				0x3120          */
/*  to:                                                                                         */
/*     #define SIS3316_ADC_CH9_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3120              */
/*                                                                                              */
/*                                                                                              */
/*  modification:    09.09.2019                                                                 */
/*  add:                              														    */
/*     #define SIS3316_EXTERNAL_VETO_GATE_DELAY_REG       		   		   		0x3C            */
/*                                                                                              */
/*     #define SIS3316_ADC_CH1_4_GATE_TEST_MODE_REG								0x10F0          */
/*     #define SIS3316_ADC_CH5_8_GATE_TEST_MODE_REG  							0x20F0          */
/*     #define SIS3316_ADC_CH9_12_GATE_TEST_MODE_REG 							0x30F0          */
/*     #define SIS3316_ADC_CH13_16_GATE_TEST_MODE_REG							0x40F0          */
/*                                                                                              */
/*                                                                                              */
/*  modification:    12.07.2021                                                                 */
/*  added:  (marked with "added 12.07.2021")												    */
/*     #define SIS3316_ADC_CH1_4_RATASER_CLOCK_MUX_SEL_REG	0x10F4    //  added 12.07.2021      */
/*      ........                                                                                */
/*      ........                                                                                */
/*                                                                                              */
/*  modification:    05.07.2024                                                                 */
/*  added:   									                                        	    */
/*     #define SIS3316_REG_I2C_PERIPHERAL_BASE	 (only SIS3316-2)                               */
/*                                                                                              */
/*                                                                                              */
/* -------------------------------------------------------------------------------------------- */
/*                                                                                              */
/*  SIS  Struck Innovative Systeme GmbH                                                         */
/*                                                                                              */
/*  Harksheider Str. 102A                                                                       */
/*  22399 Hamburg                                                                               */
/*                                                                                              */
/*  Tel. +49 (0)40 60 87 305 0                                                                  */
/*  Fax  +49 (0)40 60 87 305 20                                                                 */
/*                                                                                              */
/*  http://www.struck.de                                                                        */
/*                                                                                              */
/*  � 2024                                                                                      */
/*                                                                                              */  
/************************************************************************************************/

/*         addresses   */


#define SIS3316_CONTROL_STATUS                       					0x0			/* read/write; D32 */
#define SIS3316_MODID                                					0x4			/* read only; D32 */
#define SIS3316_IRQ_CONFIG                           					0x8			/* read/write; D32 */
#define SIS3316_IRQ_CONTROL                          					0xC			/* read/write; D32 */

#define SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL   					0x10		/* read/write; D32 */
#define SIS3316_CBLT_BROADCAST            								0x14		/* read/write; D32 */
#define SIS3316_HARDWARE_VERSION                       					0x1C			/* read only; D32 */

#define SIS3316_UDP_PROT_CONFIGURATION                 					0x8			/* read/write; D32 */



#define SIS3316_INTERNAL_TEMPERATURE_REG		      	    			0x20      /* read/write; D32 */
#define SIS3316_ONE_WIRE_CONTROL_REG       		   		    			0x24      /* read/write; D32, only SIS3316  */
#define SIS3316_REG_I2C_PERIPHERAL_BASE									0x24	  /* read/write; D32, only SIS3316-2 */
#define SIS3316_SERIAL_NUMBER_REG       		   		    			0x28      /* read only; D32 */
#define SIS3316_ADC_FPGA_BOOT_CSR                                       0x30   /* r/w; D32 */
#define SIS3316_EXTERNAL_VETO_GATE_DELAY_REG       		   		   		0x3C      /* read only; D32 */

#define SIS3316_ADC_CLK_OSC_I2C_REG				      	    			0x40      /* read/write; D32 */
#define SIS3316_MGT1_OSC_I2C_REG				      	    			0x44      /* read/write; D32 */
//#define SIS3316_REG_I2C_SI570_MGT_BASE  0x44
#define SIS3316_REG_I2C_SI570_VXS_BASE									0x48
#define SIS3316_REG_I2C_SI570_DDR3_BASE									0x4C

#define SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL      	    			0x50      /* read/write; D32 */
#define SIS3316_NIM_CLK_MULTIPLIER_SPI_REG      	    				0x54      /* read/write; D32 */
#define SIS3316_FP_LVDS_BUS_CONTROL             		    			0x58      /* read/write; D32 */
#define SIS3316_NIM_INPUT_CONTROL_REG  									0x5C      /* read/write; D32 */

#define SIS3316_ACQUISITION_CONTROL_STATUS         		    			0x60      /* read/write; D32 */

#define SIS3316_LOOKUP_TABLE_CONTROL_REG                  				0x64	 
#define SIS3316_LOOKUP_TABLE_ADDR_REG				       				0x68		 
#define SIS3316_LOOKUP_TABLE_DATA_REG				     				0x6C		 
	
#define SIS3316_LEMO_OUT_CO_SELECT_REG  								0x70      /* read/write; D32 */
#define SIS3316_LEMO_OUT_TO_SELECT_REG  								0x74      /* read/write; D32 */
#define SIS3316_LEMO_OUT_UO_SELECT_REG  								0x78      /* read/write; D32 */
#define SIS3316_INTERNAL_TRIGGER_FEEDBACK_SELECT_REG					0x7C     
                 
#define SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG							0x80  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH5_8_CTRL_REG  							0x84  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH9_12_CTRL_REG 							0x88  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH13_16_CTRL_REG  						0x8C  /* r/w; D32    name changed 31.01.2014 */

#define SIS3316_DATA_TRANSFER_ADC1_4_STATUS_REG							0x90  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC5_8_STATUS_REG  						0x94  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC9_12_STATUS_REG 						0x98  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC13_16_STATUS_REG  						0x9C  /* read; D32   name changed 31.01.2014 */

#define SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS      		    			0xA0      /* read/write; D32 */
#define SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG 							0xA4                  
#define SIS3316_VME_FPGA_SAMPLE_CLOCK_PLL_DRP_REG					    0xA8

//#define SIS3316_REG_XADC_DRP_BASE 0xA8

#define SIS3316_REG_I2C_SFP_BASE										0xB0

#define SIS3316_PRESCALER_OUTPUT_PULSE_DIVIDER_REG  					0xB8                            
#define SIS3316_PRESCALER_OUTPUT_PULSE_LENGTH_REG  						0xBC                              



#define SIS3316_KEY_RESET                           					0x400	  /* write only; D32 */
#define SIS3316_KEY_USER_FUNCTION                          				0x404	  /* write only; D32 */


//#define SIS3316_KEY_ARM                         						0x410	  /* write only; D32 */
#define SIS3316_KEY_DISARM                         						0x414	  /* write only; D32 */
#define SIS3316_KEY_TRIGGER                         					0x418	  /* write only; D32 */
#define SIS3316_KEY_TIMESTAMP_CLEAR                 					0x41C	  /* write only; D32 */
#define SIS3316_KEY_DISARM_AND_ARM_BANK1           						0x420	  /* write only; D32 */
#define SIS3316_KEY_DISARM_AND_ARM_BANK2           						0x424	  /* write only; D32 */
#define SIS3316_KEY_ENABLE_SAMPLE_BANK_SWAP_CONTROL_WITH_NIM_INPUT		0x428
#define SIS3316_KEY_DISABLE_PRESCALER_OUTPUT_PULSE_DIVIDER_LOGIC		0x42C                    

#define SIS3316_KEY_PPS_LATCH_BIT_CLEAR                 				0x430	  /* write only; D32 */

#define SIS3316_KEY_ADC_FPGA_RESET           							0x434	  /* write only; D32 */
#define SIS3316_KEY_ADC_CLOCK_DCM_RESET           						0x438	  /* write only; D32 */




/* ***************************************************************************** */

/* ADC FPGA Read/Write registers                                                 */

#define SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG     						0x1000	  
#define SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG    						0x2000	  
#define SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG   						0x3000	  
#define SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG  						0x4000	  

#define SIS3316_ADC_CH1_4_ANALOG_CTRL_REG     							0x1004	  
#define SIS3316_ADC_CH5_8_ANALOG_CTRL_REG     							0x2004	  
#define SIS3316_ADC_CH9_12_ANALOG_CTRL_REG     							0x3004	  
#define SIS3316_ADC_CH13_16_ANALOG_CTRL_REG    							0x4004	  

#define SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG 							0x1008	  
#define SIS3316_ADC_CH5_8_DAC_OFFSET_CTRL_REG  							0x2008	  
#define SIS3316_ADC_CH9_12_DAC_OFFSET_CTRL_REG 							0x3008	  
#define SIS3316_ADC_CH13_16_DAC_OFFSET_CTRL_REG							0x4008	  

#define SIS3316_ADC_CH1_4_SPI_CTRL_REG 									0x100C	  
#define SIS3316_ADC_CH5_8_SPI_CTRL_REG  								0x200C	  
#define SIS3316_ADC_CH9_12_SPI_CTRL_REG 								0x300C	  
#define SIS3316_ADC_CH13_16_SPI_CTRL_REG								0x400C	  


#define SIS3316_ADC_CH1_4_EVENT_CONFIG_REG 								0x1010	  
#define SIS3316_ADC_CH5_8_EVENT_CONFIG_REG  							0x2010	  
#define SIS3316_ADC_CH9_12_EVENT_CONFIG_REG 							0x3010	  
#define SIS3316_ADC_CH13_16_EVENT_CONFIG_REG							0x4010	  

#define SIS3316_ADC_CH1_4_EXTENDED_EVENT_CONFIG_REG 					0x109C	  
#define SIS3316_ADC_CH5_8_EXTENDED_EVENT_CONFIG_REG  					0x209C	  
#define SIS3316_ADC_CH9_12_EXTENDED_EVENT_CONFIG_REG 					0x309C	  
#define SIS3316_ADC_CH13_16_EXTENDED_EVENT_CONFIG_REG					0x409C	  


#define SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG 							0x1014	  
#define SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG  							0x2014	  
#define SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG 							0x3014	  
#define SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG							0x4014	  


#define SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG 						0x1018	  
#define SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG  						0x2018	  
#define SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG 						0x3018	  
#define SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG						0x4018	  

#define SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG 				0x101C	  
#define SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG  				0x201C	  
#define SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG 				0x301C	  
#define SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG				0x401C	  


#define SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG 					0x1020	  
#define SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG  					0x2020	  
#define SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG 					0x3020	  
#define SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG					0x4020	  

#define SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG 							0x1024	  
#define SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG  							0x2024	  
#define SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG 							0x3024	  
#define SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG							0x4024	  



#define SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG 						0x1028	  
#define SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG  						0x2028	  
#define SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG 						0x3028	  
#define SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG						0x4028	  

#define SIS3316_ADC_CH1_4_AVERAGE_CONFIGURATION_REG						0x102C // only SIS3316-16bit
#define SIS3316_ADC_CH5_8_AVERAGE_CONFIGURATION_REG						0x202C // only SIS3316-16bit
#define SIS3316_ADC_CH9_12_AVERAGE_CONFIGURATION_REG					0x302C // only SIS3316-16bit
#define SIS3316_ADC_CH13_16_AVERAGE_CONFIGURATION_REG					0x402C // only SIS3316-16bit


#define SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG 						0x1030	  
#define SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG  						0x2030	  
#define SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG 						0x3030	  
#define SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG						0x4030	  

#define SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG 					0x1034	  
#define SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG  					0x2034	  
#define SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG 					0x3034	  
#define SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG					0x4034	  



#define SIS3316_ADC_CH1_4_INTERNAL_TRIGGER_DELAY_CONFIG_REG 			0x1038	  
#define SIS3316_ADC_CH5_8_INTERNAL_TRIGGER_DELAY_CONFIG_REG				0x2038	  
#define SIS3316_ADC_CH9_12_INTERNAL_TRIGGER_DELAY_CONFIG_REG			0x3038	  
#define SIS3316_ADC_CH13_16_INTERNAL_TRIGGER_DELAY_CONFIG_REG			0x4038	  


#define SIS3316_ADC_CH1_4_INTERNAL_GATE_LENGTH_CONFIG_REG 				0x103C	  
#define SIS3316_ADC_CH5_8_INTERNAL_GATE_LENGTH_CONFIG_REG				0x203C	  
#define SIS3316_ADC_CH9_12_INTERNAL_GATE_LENGTH_CONFIG_REG				0x303C	  
#define SIS3316_ADC_CH13_16_INTERNAL_GATE_LENGTH_CONFIG_REG				0x403C	  



#define SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG							0x1040 
#define SIS3316_ADC_CH2_FIR_TRIGGER_SETUP_REG							0x1050 
#define SIS3316_ADC_CH3_FIR_TRIGGER_SETUP_REG							0x1060 
#define SIS3316_ADC_CH4_FIR_TRIGGER_SETUP_REG							0x1070 

#define SIS3316_ADC_CH5_FIR_TRIGGER_SETUP_REG							0x2040 
#define SIS3316_ADC_CH6_FIR_TRIGGER_SETUP_REG							0x2050 
#define SIS3316_ADC_CH7_FIR_TRIGGER_SETUP_REG							0x2060 
#define SIS3316_ADC_CH8_FIR_TRIGGER_SETUP_REG							0x2070 

#define SIS3316_ADC_CH9_FIR_TRIGGER_SETUP_REG							0x3040 
#define SIS3316_ADC_CH10_FIR_TRIGGER_SETUP_REG							0x3050 
#define SIS3316_ADC_CH11_FIR_TRIGGER_SETUP_REG							0x3060 
#define SIS3316_ADC_CH12_FIR_TRIGGER_SETUP_REG							0x3070 

#define SIS3316_ADC_CH13_FIR_TRIGGER_SETUP_REG							0x4040 
#define SIS3316_ADC_CH14_FIR_TRIGGER_SETUP_REG							0x4050 
#define SIS3316_ADC_CH15_FIR_TRIGGER_SETUP_REG							0x4060 
#define SIS3316_ADC_CH16_FIR_TRIGGER_SETUP_REG							0x4070 

#define SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG						0x1080 
#define SIS3316_ADC_CH5_8_SUM_FIR_TRIGGER_SETUP_REG						0x2080 
#define SIS3316_ADC_CH9_12_SUM_FIR_TRIGGER_SETUP_REG					0x3080 
#define SIS3316_ADC_CH13_16_SUM_FIR_TRIGGER_SETUP_REG					0x4080 




#define SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG						0x1044 
#define SIS3316_ADC_CH2_FIR_TRIGGER_THRESHOLD_REG						0x1054 
#define SIS3316_ADC_CH3_FIR_TRIGGER_THRESHOLD_REG						0x1064 
#define SIS3316_ADC_CH4_FIR_TRIGGER_THRESHOLD_REG						0x1074 

#define SIS3316_ADC_CH5_FIR_TRIGGER_THRESHOLD_REG						0x2044 
#define SIS3316_ADC_CH6_FIR_TRIGGER_THRESHOLD_REG						0x2054 
#define SIS3316_ADC_CH7_FIR_TRIGGER_THRESHOLD_REG						0x2064 
#define SIS3316_ADC_CH8_FIR_TRIGGER_THRESHOLD_REG						0x2074 

#define SIS3316_ADC_CH9_FIR_TRIGGER_THRESHOLD_REG						0x3044 
#define SIS3316_ADC_CH10_FIR_TRIGGER_THRESHOLD_REG						0x3054 
#define SIS3316_ADC_CH11_FIR_TRIGGER_THRESHOLD_REG						0x3064 
#define SIS3316_ADC_CH12_FIR_TRIGGER_THRESHOLD_REG						0x3074 

#define SIS3316_ADC_CH13_FIR_TRIGGER_THRESHOLD_REG						0x4044 
#define SIS3316_ADC_CH14_FIR_TRIGGER_THRESHOLD_REG						0x4054 
#define SIS3316_ADC_CH15_FIR_TRIGGER_THRESHOLD_REG						0x4064 
#define SIS3316_ADC_CH16_FIR_TRIGGER_THRESHOLD_REG						0x4074 

#define SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG					0x1084 
#define SIS3316_ADC_CH5_8_SUM_FIR_TRIGGER_THRESHOLD_REG					0x2084 
#define SIS3316_ADC_CH9_12_SUM_FIR_TRIGGER_THRESHOLD_REG				0x3084 
#define SIS3316_ADC_CH13_16_SUM_FIR_TRIGGER_THRESHOLD_REG				0x4084 


#define SIS3316_ADC_CH1_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1048 
#define SIS3316_ADC_CH2_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1058 
#define SIS3316_ADC_CH3_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1068 
#define SIS3316_ADC_CH4_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1078 

#define SIS3316_ADC_CH5_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2048 
#define SIS3316_ADC_CH6_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2058 
#define SIS3316_ADC_CH7_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2068 
#define SIS3316_ADC_CH8_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2078 

#define SIS3316_ADC_CH9_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3048 
#define SIS3316_ADC_CH10_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3058 
#define SIS3316_ADC_CH11_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3068 
#define SIS3316_ADC_CH12_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3078 

#define SIS3316_ADC_CH13_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4048 
#define SIS3316_ADC_CH14_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4058 
#define SIS3316_ADC_CH15_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4068 
#define SIS3316_ADC_CH16_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4078 

#define SIS3316_ADC_CH1_4_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG				0x1088 
#define SIS3316_ADC_CH5_8_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG				0x2088 
#define SIS3316_ADC_CH9_12_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG			0x3088 
#define SIS3316_ADC_CH13_16_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG			0x4088 



#define SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG			0x1090 
#define SIS3316_ADC_CH5_8_TRIGGER_STATISTIC_COUNTER_MODE_REG			0x2090 
#define SIS3316_ADC_CH9_12_TRIGGER_STATISTIC_COUNTER_MODE_REG			0x3090 
#define SIS3316_ADC_CH13_16_TRIGGER_STATISTIC_COUNTER_MODE_REG			0x4090 


#define SIS3316_ADC_CH1_4_PEAK_CHARGE_CONFIGURATION_REG 				0x1094	  
#define SIS3316_ADC_CH5_8_PEAK_CHARGE_CONFIGURATION_REG     			0x2094	  
#define SIS3316_ADC_CH9_12_PEAK_CHARGE_CONFIGURATION_REG    			0x3094	  
#define SIS3316_ADC_CH13_16_PEAK_CHARGE_CONFIGURATION_REG 				0x4094

#define SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x1098	  
#define SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x2098	  
#define SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x3098	  
#define SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG			0x4098	  


#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG 					0x10A0	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE1_CONFIG_REG  				0x20A0	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE1_CONFIG_REG 				0x30A0	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE1_CONFIG_REG				0x40A0	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE2_CONFIG_REG 					0x10A4	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE2_CONFIG_REG  				0x20A4	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE2_CONFIG_REG 				0x30A4	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE2_CONFIG_REG				0x40A4	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE3_CONFIG_REG 					0x10A8	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE3_CONFIG_REG  				0x20A8	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE3_CONFIG_REG 				0x30A8	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE3_CONFIG_REG				0x40A8	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE4_CONFIG_REG 					0x10AC	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE4_CONFIG_REG  				0x20AC	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE4_CONFIG_REG 				0x30AC	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE4_CONFIG_REG				0x40AC	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE5_CONFIG_REG 					0x10B0	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE5_CONFIG_REG  				0x20B0	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE5_CONFIG_REG 				0x30B0	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE5_CONFIG_REG				0x40B0	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE6_CONFIG_REG 					0x10B4	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE6_CONFIG_REG  				0x2004	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE6_CONFIG_REG 				0x30B4	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE6_CONFIG_REG				0x40B4	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE7_CONFIG_REG 					0x10B8	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE7_CONFIG_REG  				0x20B8	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE7_CONFIG_REG 				0x30B8	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE7_CONFIG_REG				0x40B8	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE8_CONFIG_REG 					0x10BC	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE8_CONFIG_REG  				0x20BC	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE8_CONFIG_REG 				0x30BC	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE8_CONFIG_REG				0x40BC	  



#define SIS3316_ADC_CH1_FIR_ENERGY_SETUP_REG							0x10C0    //  add 30.05.2013
#define SIS3316_ADC_CH2_FIR_ENERGY_SETUP_REG							0x10C4    //  add 30.05.2013
#define SIS3316_ADC_CH3_FIR_ENERGY_SETUP_REG							0x10C8    //  add 30.05.2013
#define SIS3316_ADC_CH4_FIR_ENERGY_SETUP_REG							0x10CC    //  add 30.05.2013

#define SIS3316_ADC_CH5_FIR_ENERGY_SETUP_REG							0x20C0    //  add 30.05.2013
#define SIS3316_ADC_CH6_FIR_ENERGY_SETUP_REG							0x20C4    //  add 30.05.2013
#define SIS3316_ADC_CH7_FIR_ENERGY_SETUP_REG							0x20C8    //  add 30.05.2013
#define SIS3316_ADC_CH8_FIR_ENERGY_SETUP_REG							0x20CC    //  add 30.05.2013

#define SIS3316_ADC_CH9_FIR_ENERGY_SETUP_REG							0x30C0    //  add 30.05.2013
#define SIS3316_ADC_CH10_FIR_ENERGY_SETUP_REG							0x30C4    //  add 30.05.2013
#define SIS3316_ADC_CH11_FIR_ENERGY_SETUP_REG							0x30C8    //  add 30.05.2013
#define SIS3316_ADC_CH12_FIR_ENERGY_SETUP_REG							0x30CC    //  add 30.05.2013

#define SIS3316_ADC_CH13_FIR_ENERGY_SETUP_REG							0x40C0    //  add 30.05.2013
#define SIS3316_ADC_CH14_FIR_ENERGY_SETUP_REG							0x40C4    //  add 30.05.2013
#define SIS3316_ADC_CH15_FIR_ENERGY_SETUP_REG							0x40C8    //  add 30.05.2013
#define SIS3316_ADC_CH16_FIR_ENERGY_SETUP_REG							0x40CC    //  add 30.05.2013


#define SIS3316_ADC_CH1_HISTOGRAM_CONF_REG								0x10D0    //  add 30.10.2013		
#define SIS3316_ADC_CH2_HISTOGRAM_CONF_REG								0x10D4    //  add 30.10.2013
#define SIS3316_ADC_CH3_HISTOGRAM_CONF_REG								0x10D8    //  add 30.10.2013
#define SIS3316_ADC_CH4_HISTOGRAM_CONF_REG								0x10DC    //  add 30.10.2013
 
#define SIS3316_ADC_CH5_HISTOGRAM_CONF_REG								0x20D0    //  add 30.10.2013		
#define SIS3316_ADC_CH6_HISTOGRAM_CONF_REG								0x20D4    //  add 30.10.2013
#define SIS3316_ADC_CH7_HISTOGRAM_CONF_REG								0x20D8    //  add 30.10.2013
#define SIS3316_ADC_CH8_HISTOGRAM_CONF_REG								0x20DC    //  add 30.10.2013
 
#define SIS3316_ADC_CH9_HISTOGRAM_CONF_REG								0x30D0    //  add 30.10.2013		
#define SIS3316_ADC_CH10_HISTOGRAM_CONF_REG								0x30D4    //  add 30.10.2013
#define SIS3316_ADC_CH11_HISTOGRAM_CONF_REG								0x30D8    //  add 30.10.2013
#define SIS3316_ADC_CH12_HISTOGRAM_CONF_REG								0x30DC    //  add 30.10.2013
 
#define SIS3316_ADC_CH13_HISTOGRAM_CONF_REG								0x40D0    //  add 30.10.2013		
#define SIS3316_ADC_CH14_HISTOGRAM_CONF_REG								0x40D4    //  add 30.10.2013
#define SIS3316_ADC_CH15_HISTOGRAM_CONF_REG								0x40D8    //  add 30.10.2013
#define SIS3316_ADC_CH16_HISTOGRAM_CONF_REG								0x40DC    //  add 30.10.2013



#define SIS3316_ADC_CH1_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x10E0	  //  add 25.06.2015
#define SIS3316_ADC_CH2_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x10E4	  //  add 25.06.2015
#define SIS3316_ADC_CH3_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x10E8	  //  add 25.06.2015
#define SIS3316_ADC_CH4_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x10EC	  //  add 25.06.2015

#define SIS3316_ADC_CH5_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x20E0	  //  add 25.06.2015
#define SIS3316_ADC_CH6_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x20E4	  //  add 25.06.2015
#define SIS3316_ADC_CH7_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x20E8	  //  add 25.06.2015
#define SIS3316_ADC_CH8_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x20EC	  //  add 25.06.2015

#define SIS3316_ADC_CH9_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x30E0	  //  add 25.06.2015
#define SIS3316_ADC_CH10_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x30E4	  //  add 25.06.2015
#define SIS3316_ADC_CH11_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x30E8	  //  add 25.06.2015
#define SIS3316_ADC_CH12_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG 		0x30EC	  //  add 25.06.2015

#define SIS3316_ADC_CH13_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG		0x40E0	  //  add 25.06.2015
#define SIS3316_ADC_CH14_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG		0x40E4	  //  add 25.06.2015
#define SIS3316_ADC_CH15_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG		0x40E8	  //  add 25.06.2015
#define SIS3316_ADC_CH16_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG		0x40EC	  //  add 25.06.2015

#define SIS3316_ADC_CH1_4_GATE_TEST_MODE_REG							0x10F0	  
#define SIS3316_ADC_CH5_8_GATE_TEST_MODE_REG  							0x20F0	  
#define SIS3316_ADC_CH9_12_GATE_TEST_MODE_REG 							0x30F0	  
#define SIS3316_ADC_CH13_16_GATE_TEST_MODE_REG							0x40F0	  


#define SIS3316_ADC_CH1_4_RATASER_CLOCK_MUX_SEL_REG						0x10F4    //  added 12.07.2021
#define SIS3316_ADC_CH5_8_RATASER_CLOCK_MUX_SEL_REG						0x20F4
#define SIS3316_ADC_CH9_12_RATASER_CLOCK_MUX_SEL_REG					0x30F4
#define SIS3316_ADC_CH13_16_RATASER_CLOCK_MUX_SEL_REG					0x40F4

#define SIS3316_ADC_CH1_4_RATASER_CLOCK_CONTROL1_REG					0x10F8	//  added 12.07.2021
#define SIS3316_ADC_CH5_8_RATASER_CLOCK_CONTROL1_REG					0x20F8
#define SIS3316_ADC_CH9_12_RATASER_CLOCK_CONTROL1_REG					0x30F8
#define SIS3316_ADC_CH13_16_RATASER_CLOCK_CONTROL1_REG					0x40F8

#define SIS3316_ADC_CH1_4_RATASER_CLOCK_CONTROL2_REG					0x10FC	//  added 12.07.2021
#define SIS3316_ADC_CH5_8_RATASER_CLOCK_CONTROL2_REG					0x20FC
#define SIS3316_ADC_CH9_12_RATASER_CLOCK_CONTROL2_REG					0x30FC
#define SIS3316_ADC_CH13_16_RATASER_CLOCK_CONTROL2_REG					0x40FC

/* ***************************************************************************** */

/* ADC FPGA Read registers                                                       */

#define SIS3316_ADC_CH1_4_FIRMWARE_REG     								0x1100 /* rd only */  
#define SIS3316_ADC_CH5_8_FIRMWARE_REG    								0x2100 /* rd only */
#define SIS3316_ADC_CH9_12_FIRMWARE_REG   								0x3100 /* rd only */
#define SIS3316_ADC_CH13_16_FIRMWARE_REG  								0x4100 /* rd only */

#define SIS3316_ADC_CH1_4_STATUS_REG     								0x1104 /* rd only */  
#define SIS3316_ADC_CH5_8_STATUS_REG    								0x2104 /* rd only */
#define SIS3316_ADC_CH9_12_STATUS_REG   								0x3104 /* rd only */
#define SIS3316_ADC_CH13_16_STATUS_REG  								0x4104 /* rd only */

#define SIS3316_ADC_CH1_4_DAC_OFFSET_READBACK_REG 						0x1108	  
#define SIS3316_ADC_CH5_8_DAC_OFFSET_READBACK_REG 						0x2108	  
#define SIS3316_ADC_CH9_12_DAC_OFFSET_READBACK_REG						0x3108	  
#define SIS3316_ADC_CH13_16_DAC_OFFSET_READBACK_REG						0x4108	  

#define SIS3316_ADC_CH1_4_SPI_READBACK_REG 								0x110C	  
#define SIS3316_ADC_CH5_8_SPI_READBACK_REG  							0x210C	  
#define SIS3316_ADC_CH9_12_SPI_READBACK_REG 							0x310C	  
#define SIS3316_ADC_CH13_16_SPI_READBACK_REG							0x410C	  




#define SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG						0x1110 /* rd only */  
#define SIS3316_ADC_CH2_ACTUAL_SAMPLE_ADDRESS_REG						0x1114 /* rd only */  
#define SIS3316_ADC_CH3_ACTUAL_SAMPLE_ADDRESS_REG						0x1118 /* rd only */  
#define SIS3316_ADC_CH4_ACTUAL_SAMPLE_ADDRESS_REG						0x111C /* rd only */  

#define SIS3316_ADC_CH5_ACTUAL_SAMPLE_ADDRESS_REG						0x2110 /* rd only */  
#define SIS3316_ADC_CH6_ACTUAL_SAMPLE_ADDRESS_REG						0x2114 /* rd only */  
#define SIS3316_ADC_CH7_ACTUAL_SAMPLE_ADDRESS_REG						0x2118 /* rd only */  
#define SIS3316_ADC_CH8_ACTUAL_SAMPLE_ADDRESS_REG						0x211C /* rd only */  

#define SIS3316_ADC_CH9_ACTUAL_SAMPLE_ADDRESS_REG						0x3110 /* rd only */  
#define SIS3316_ADC_CH10_ACTUAL_SAMPLE_ADDRESS_REG						0x3114 /* rd only */  
#define SIS3316_ADC_CH11_ACTUAL_SAMPLE_ADDRESS_REG						0x3118 /* rd only */  
#define SIS3316_ADC_CH12_ACTUAL_SAMPLE_ADDRESS_REG						0x311C /* rd only */  

#define SIS3316_ADC_CH13_ACTUAL_SAMPLE_ADDRESS_REG						0x4110 /* rd only */  
#define SIS3316_ADC_CH14_ACTUAL_SAMPLE_ADDRESS_REG						0x4114 /* rd only */  
#define SIS3316_ADC_CH15_ACTUAL_SAMPLE_ADDRESS_REG						0x4118 /* rd only */  
#define SIS3316_ADC_CH16_ACTUAL_SAMPLE_ADDRESS_REG						0x411C /* rd only */  



#define SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1120 
#define SIS3316_ADC_CH2_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1124    
#define SIS3316_ADC_CH3_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1128    
#define SIS3316_ADC_CH4_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x112C    

#define SIS3316_ADC_CH5_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2120    
#define SIS3316_ADC_CH6_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2124    
#define SIS3316_ADC_CH7_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2128    
#define SIS3316_ADC_CH8_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x212C    

#define SIS3316_ADC_CH9_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3120    
#define SIS3316_ADC_CH10_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3124    
#define SIS3316_ADC_CH11_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3128    
#define SIS3316_ADC_CH12_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x312C    

#define SIS3316_ADC_CH13_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4120    
#define SIS3316_ADC_CH14_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4124    
#define SIS3316_ADC_CH15_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4128    
#define SIS3316_ADC_CH16_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x412C    



#define SIS3316_ADC_CH1_4_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x1130	  
#define SIS3316_ADC_CH1_4_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x1134	  
#define SIS3316_ADC_CH5_8_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x2130	  
#define SIS3316_ADC_CH5_8_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x2134	  

#define SIS3316_ADC_CH9_12_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x3130	  
#define SIS3316_ADC_CH9_12_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x3134	  
#define SIS3316_ADC_CH13_16_PPS_UPPER_TIMESTAMP_LATCH_REG 				0x4130	  
#define SIS3316_ADC_CH13_16_PPS_LOWER_TIMESTAMP_LATCH_REG 				0x4134	  



#define SIS3316_ADC_CH1_4_RATASER_CLOCK_STATUS1_REG						0x1130	//  added 12.07.2021
#define SIS3316_ADC_CH5_8_RATASER_CLOCK_STATUS1_REG						0x2130
#define SIS3316_ADC_CH9_12_RATASER_CLOCK_STATUS1_REG					0x3130
#define SIS3316_ADC_CH13_16_RATASER_CLOCK_STATUS1_REG					0x4130




/* ***************************************************************************** */

#define SIS3316_FPGA_ADC1_REG_BASE					  					0x1000	  
#define SIS3316_FPGA_ADC2_REG_BASE					  					0x2000	  
#define SIS3316_FPGA_ADC3_REG_BASE					  					0x3000	  
#define SIS3316_FPGA_ADC4_REG_BASE					  					0x4000	  

#define SIS3316_FPGA_ADC_REG_OFFSET					  					0x1000	  

// Actually the below are group FIFOS.

#define SIS3316_FPGA_ADC1_MEM_BASE					  					0x100000	  
#define SIS3316_FPGA_ADC2_MEM_BASE					  					0x200000	  
#define SIS3316_FPGA_ADC3_MEM_BASE					  					0x300000	  
#define SIS3316_FPGA_ADC4_MEM_BASE					  					0x400000	  

#define SIS3316_FPGA_ADC_MEM_OFFSET					  					0x100000	  

#define SIS3316_ADC_MEMORY_BANK_32BIT_SIZE				  				0x0FEFFFF	  
#define MAX_NUMBER_LWORDS_64MBYTE										0x1000000       /* 64MByte */


/* ***************************************************************************** */
// add 30.05.2013
#define FIR_ENERGY_MAX_PEAKING			0x7fe
#define FIR_ENERGY_MIN_PEAKING			2

#define FIR_ENERGY_MAX_GAP				0x1fe
#define FIR_ENERGY_MIN_GAP				2

#define FIR_ENERGY_MAX_TAU_TABLE		0x3
#define FIR_ENERGY_MAX_TAU_FACTOR		0x3f
#define FIR_ENERGY_MAX_ADD_AVERAGE		3

// add 30.05.2013
#define ACCUMULATOR_MAX_START_INDEX		0xFFFF
#define ACCUMULATOR_MAX_LENGTH			0x1FF
