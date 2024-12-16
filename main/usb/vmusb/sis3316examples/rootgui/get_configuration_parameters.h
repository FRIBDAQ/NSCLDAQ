/***************************************************************************/
/*                                                                         */
/*  Filename: get_configuration_parameters.h                               */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    08.11.2021                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  © 2021                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef GET_CONFIGURATION_PARAMETERS_H_
#define GET_CONFIGURATION_PARAMETERS_H_

/*  SIS3316_root_gui configuration parameters   */


#define TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_ENABLE 		"PARAMETER_STOP_AFTER_TIME_SECONDS_ENABLE"
#define TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_VALUE 		"PARAMETER_STOP_AFTER_TIME_SECONDS_VALUE"


#define TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_ENABLE 			"PARAMETER_STOP_AFTER_NOF_LOOPS_ENABLE"
#define TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_VALUE 			"PARAMETER_STOP_AFTER_NOF_LOOPS_VALUE"

 
#define TEXT_PARAMETER_SAMPLE_CONTROL_MODE 					"PARAMETER_SAMPLE_CONTROL_MODE"
#define TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE 		"PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE"
#define TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE 		"PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE"


#define TEXT_PARAMETER_PRE_TRIGGER_DELAY   					"PARAMETER_PRE_TRIGGER_DELAY"
#define TEXT_PARAMETER_RAW_SAMPLE_START_INDEX				"PARAMETER_RAW_SAMPLE_START_INDEX"
#define TEXT_PARAMETER_RAW_SAMPLE_LENGTH   					"PARAMETER_RAW_SAMPLE_LENGTH"


#define TEXT_PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG 	"PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG"
#define TEXT_PARAMETER_MAW_TEST_BUFFER_LENGTH   			"PARAMETER_MAW_TEST_BUFFER_LENGTH"
#define TEXT_PARAMETER_MAW_TEST_BUFFER_DELAY   				"PARAMETER_MAW_TEST_BUFFER_DELAY"
#define TEXT_PARAMETER_MAW_TEST_START_INDEX					"PARAMETER_MAW_TEST_START_INDEX"



#define TEXT_PARAMETER_SUPPRESS_EVENTS_IF_ADDR_THRES_ENABLE	"PARAMETER_SUPPRESS_EVENTS_IF_ADDR_THRES_ENABLE"

#define TEXT_PARAMETER_FORMAT_ACCUM_ENABLE					"PARAMETER_FORMAT_ACCUM_ENABLE"
#define TEXT_PARAMETER_FORMAT_ACCUM78_ENABLE				"PARAMETER_FORMAT_ACCUM78_ENABLE"
#define TEXT_PARAMETER_FORMAT_MAW_ENABLE					"PARAMETER_FORMAT_MAW_ENABLE"
#define TEXT_PARAMETER_FORMAT_ENERGY_ENABLE					"PARAMETER_FORMAT_ENERGY_ENABLE"
#define TEXT_PARAMETER_SAVE_RAW_DATA_FIRST_EVENT_ENABLE		"PARAMETER_SAVE_RAW_DATA_FIRST_EVENT_ENABLE"


#define TEXT_PARAMETER_EXT_TRIGGER_FUNC_AS_EXT_TRIGGER		"PARAMETER_EXT_TRIGGER_FUNC_AS_EXT_TRIGGER"
#define TEXT_PARAMETER_TRIGGER_COND_SOFTWARE_KEY			"PARAMETER_TRIGGER_COND_SOFTWARE_KEY"
#define TEXT_PARAMETER_TRIGGER_COND_VME_LEMO_TI				"PARAMETER_TRIGGER_COND_VME_LEMO_TI"

#define TEXT_PARAMETER_EXT_TRIGGER_DISABLE_WITH_BUSY		"PARAMETER_EXT_TRIGGER_DISABLE_WITH_BUSY"
#define TEXT_PARAMETER_FEEDBACK_INT_TRIGGER_AS_EXT_TRIGGER	"PARAMETER_FEEDBACK_INT_TRIGGER_AS_EXT_TRIGGER"


#define TEXT_PARAMETER_SELECT_FEEDBACK_COINCIDENCE1_TRIGGER	"PARAMETER_SELECT_FEEDBACK_COINCIDENCE1_TRIGGER"
#define TEXT_PARAMETER_CHANNEL_SELECT_FEEDBACK_INT_TRIGGER	"PARAMETER_CHANNEL_SELECT_FEEDBACK_INT_TRIGGER"

#define TEXT_PARAMETER_EXT_TRIG_FUNC_AS_EXT_VETO_GATE		"PARAMETER_EXT_TRIG_FUNC_AS_EXT_VETO_GATE"
#define TEXT_PARAMETER_LOCAL_VETO_FUNC_AS_EXT_VETO_GATE		"PARAMETER_LOCAL_VETO_FUNC_AS_EXT_VETO_GATE"
#define TEXT_PARAMETER_UI_AS_LOCAL_VETO_FUNC				"PARAMETER_UI_AS_LOCAL_VETO_FUNC"

#define TEXT_PARAMETER_EXT_VETO_DELAY						"PARAMETER_EXT_VETO_DELAY"
#define TEXT_PARAMETER_CHANNEL_INT_TRIGGER_DELAY			"PARAMETER_CHANNEL_INT_TRIGGER_DELAY"


#define TEXT_PARAMETER_CHANNEL_EXT_TRIGGER_ENBALE			"PARAMETER_CHANNEL_EXT_TRIGGER_ENBALE"
#define TEXT_PARAMETER_CHANNEL_INT_TRIGGER_ENBALE			"PARAMETER_CHANNEL_INT_TRIGGER_ENBALE"
#define TEXT_PARAMETER_CHANNEL_INT_SUM_TRIGGER_ENBALE		"PARAMETER_CHANNEL_INT_SUM_TRIGGER_ENBALE"
#define TEXT_PARAMETER_CHANNEL_INT_PILEUP_TRIGGER_ENBALE	"PARAMETER_CHANNEL_INT_PILEUP_TRIGGER_ENBALE"
#define TEXT_PARAMETER_CHANNEL_EXT_GATE_ENBALE				"PARAMETER_CHANNEL_EXT_GATE_ENBALE"
#define TEXT_PARAMETER_CHANNEL_EXT_VETO_ENBALE				"PARAMETER_CHANNEL_EXT_VETO_ENBALE"


// -------- Display
#define TEXT_PARAMETER_DISPLAY_CHANNEL_ENABLE 				"PARAMETER_DISPLAY_CHANNEL_ENABLE"
#define TEXT_PARAMETER_DISPLAY_STATISTIC_COUNTER_ENABLE 	"PARAMETER_DISPLAY_STATISTIC_COUNTER_ENABLE"
#define TEXT_PARAMETER_DISPLAY_MAW_SELECT 					"PARAMETER_DISPLAY_MAW_SELECT"
#define TEXT_PARAMETER_DISPLAY_HISTOGRAM_SELECT 			"PARAMETER_DISPLAY_HISTOGRAM_SELECT"
#define TEXT_PARAMETER_DISPLAY_FFT_SELECT 					"PARAMETER_DISPLAY_FFT_SELECT"


#define TEXT_PARAMETER_CHANNEL_POLARITY_INVERT   			"PARAMETER_CHANNEL_POLARITY_INVERT"
#define TEXT_PARAMETER_CHANNEL_RANGE_2V   					"PARAMETER_CHANNEL_RANGE_2V"
#define TEXT_PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE   	"PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE"
#define TEXT_PARAMETER_CHANNEL_ADC_OFFSET  					"PARAMETER_CHANNEL_ADC_OFFSET"



// --------
#define TEXT_PARAMETER_LEMO_OUT_CO_SELECT  					"PARAMETER_LEMO_OUT_CO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_TO_SELECT  					"PARAMETER_LEMO_OUT_TO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_UO_SELECT  					"PARAMETER_LEMO_OUT_UO_SELECT"

// --------


#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE   		"PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_CFD_ENABLE    "PARAMETER_CHANNEL_TRIGGER_GENERATION_CFD_ENABLE "
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD    	"PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD "
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING   	"PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP			"PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH	"PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH"

#define TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_THRESHOLD  "PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_THRESHOLD"
#define TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_SUPPRESS	"PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_SUPPRESS"
#define TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_CFD_FUNCTION_IDX "PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_CFD_FUNCTION_IDX"

#define TEXT_PARAMETER_SUM_TRIGGER_0_3		"PARAMETER_SUM_TRIGGER_0_3"
#define TEXT_PARAMETER_SUM_TRIGGER_4_7		"PARAMETER_SUM_TRIGGER_4_7"
#define TEXT_PARAMETER_SUM_TRIGGER_8_11		"PARAMETER_SUM_TRIGGER_8_11"
#define TEXT_PARAMETER_SUM_TRIGGER_12_15	"PARAMETER_SUM_TRIGGER_12_15"

#define TEXT_PARAMETER_PILEUP_WINDOW_LENGTH   				"PARAMETER_PILEUP_WINDOW_LENGTH"
#define TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH   			"PARAMETER_REPILEUP_WINDOW_LENGTH"


#define TEXT_PARAMETER_VME_TRIGGER_IDX		"PARAMETER_VME_TRIGGER_IDX"
#define TEXT_PARAMETER_VME_HE_TRIGGER_IDX	"PARAMETER_VME_HE_TRIGGER_IDX"

// --------

#define TEXT_PARAMETER_ENERGY_PEAKING			"PARAMETER_ENERGY_PEAKING"
#define TEXT_PARAMETER_ENERGY_GAP				"PARAMETER_ENERGY_GAP"
#define TEXT_PARAMETER_ENERGY_DECAY_TAU_TABLE	"PARAMETER_ENERGY_DECAY_TAU_TABLE"
#define TEXT_PARAMETER_ENERGY_DECAY_TAU_FACTOR	"PARAMETER_ENERGY_DECAY_TAU_FACTOR"
#define TEXT_PARAMETER_ENERGY_AVG_FACTOR		"PARAMETER_ENERGY_AVG_FACTOR"
#define TEXT_PARAMETER_ENERGY_PICKUP_IDX		"PARAMETER_ENERGY_PICKUP_IDX"

#define TEXT_PARAMETER_GATE1_START_INDEX	"PARAMETER_GATE1_START_INDEX"
#define TEXT_PARAMETER_GATE1_LENGTH			"PARAMETER_GATE1_LENGTH"
#define TEXT_PARAMETER_GATE2_START_INDEX	"PARAMETER_GATE2_START_INDEX"
#define TEXT_PARAMETER_GATE2_LENGTH			"PARAMETER_GATE2_LENGTH"
#define TEXT_PARAMETER_GATE3_START_INDEX	"PARAMETER_GATE3_START_INDEX"
#define TEXT_PARAMETER_GATE3_LENGTH			"PARAMETER_GATE3_LENGTH"
#define TEXT_PARAMETER_GATE4_START_INDEX	"PARAMETER_GATE4_START_INDEX"
#define TEXT_PARAMETER_GATE4_LENGTH			"PARAMETER_GATE4_LENGTH"
#define TEXT_PARAMETER_GATE5_START_INDEX	"PARAMETER_GATE5_START_INDEX"
#define TEXT_PARAMETER_GATE5_LENGTH			"PARAMETER_GATE5_LENGTH"
#define TEXT_PARAMETER_GATE6_START_INDEX	"PARAMETER_GATE6_START_INDEX"
#define TEXT_PARAMETER_GATE6_LENGTH			"PARAMETER_GATE6_LENGTH"


#define TEXT_PARAMETER_GATE7_START_INDEX	"PARAMETER_GATE7_START_INDEX"
#define TEXT_PARAMETER_GATE7_LENGTH			"PARAMETER_GATE7_LENGTH"
#define TEXT_PARAMETER_GATE8_START_INDEX	"PARAMETER_GATE8_START_INDEX"
#define TEXT_PARAMETER_GATE8_LENGTH			"PARAMETER_GATE8_LENGTH"


// --------
#define TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_IDX	"PARAMETER_INTERNAL_SAMPLE_CLOCK_IDX"
#define TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_FP_EN	"PARAMETER_INTERNAL_SAMPLE_CLOCK_FP_EN"
#define TEXT_PARAMETER_FP_CLOCK_IDX					"PARAMETER_FP_CLOCK_IDX"
#define TEXT_PARAMETER_SAMPLE_CLOCK_IDX				"PARAMETER_SAMPLE_CLOCK_IDX"
#define TEXT_PARAMETER_MULTIPLIER_IDX				"PARAMETER_MULTIPLIER_IDX"
#define TEXT_PARAMETER_COINCIDENCE_LOOKUP_TABLE_IDX	"PARAMETER_COINCIDENCE_LOOKUP_TABLE_IDX"

// --------
#define TEXT_PARAMETER_LEMO_OUT_CO_SELECT	"PARAMETER_LEMO_OUT_CO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_TO_SELECT	"PARAMETER_LEMO_OUT_TO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_UO_SELECT	"PARAMETER_LEMO_OUT_UO_SELECT"

// --------



class sis3316_get_configuration_parameters
{

private:
	int getNofKewWordLetters (char* line_buf, int max_length);
	int getFollowingParameter(char* line_buf, int max_length);
	int getFollowingChannelParameters(char* line_buf, int max_length, unsigned int* channel_parameters );

	bool isKeyWord (char c ) ;

	bool isUpperLetter (char c );
	bool isDigit (char c );
	bool isControl (char c );
	bool isHexDigit (char c );

public:

	//unsigned int uint_nof_events ;

	unsigned int uint_stop_after_time_seconds_enable;
	unsigned int uint_stop_aftertime_seconds_value;

	unsigned int uint_stop_after_nof_loops_enable ;
	unsigned int uint_stop_after_nof_loops_value ;

	unsigned int uint_sample_control_mode ;
	unsigned int uint_sample_control_use_max_enable ;
	unsigned int uint_sample_control_use_max_value ;


	unsigned int uint_pre_trigger_delay ;
	unsigned int uint_raw_sample_start_index;
	unsigned int uint_raw_sample_length ;


	unsigned int uint_maw_test_buffer_select_energy_flag ;
	unsigned int uint_maw_test_buffer_length ;
	unsigned int uint_maw_test_buffer_delay ;
	unsigned int uint_maw_test_start_index;

	unsigned int uint_SuppressEventsIfAddrThres_enable_flag;

	unsigned int uint_format_accum_enable_flag;
	unsigned int uint_format_accum78_enable_flag;
	unsigned int uint_format_maw_enable_flag;
	unsigned int uint_format_energy_enable_flag;
	unsigned int uint_SaveRawDataFirstEventOnly_enable_flag;
	

	unsigned int uint_ext_trigger_function_as_extTrigger_enable_flag;
	unsigned int uint_trigger_cond_software_key_flag;
	unsigned int uint_trigger_cond_vme_lemo_ti_flag;
	unsigned int uint_extTrig_disable_with_Busy_flag;
	unsigned int uint_feedback_intTrig_as_extTrig_flag;
	
	unsigned int uint_Coincidence1FeedbackTrigger_enable;
	unsigned int uint_IntFeedbackTrigger_enable[16];

	unsigned int uint_ext_trigger_function_as_Veto_enable_flag;
	unsigned int uint_local_veto_function_as_Veto_enable_flag;
	unsigned int uint_LemoInUI_as_local_veto_function_enable_flag;

	unsigned int uint_ext_Veto_delay;
	unsigned int uint_int_Trigger_delay[16];


	unsigned int uint_ExtTrigger_enable[16];
	unsigned int uint_IntTrigger_enable[16];
	unsigned int uint_SumTrigger_enable[16];
	unsigned int uint_IntPileupTrigger_enable[16];

	unsigned int uint_ExtGate_enable[16];
	unsigned int uint_ExtVeto_enable[16];


	// -------- Display
	unsigned int uint_DisplayChannel_enable[16];
	unsigned int uint_DisplayStatisticCounter_enable;
	unsigned int uint_DisplayMAW_select;
	unsigned int uint_DisplayHistogram_select;
	unsigned int uint_DisplayFFT_select;


// -----------

	unsigned int uint_channel_polarity_invert[16] ;

	unsigned int uint_channel_range_2V[16] ;
	unsigned int uint_channel_50ohm_termination_disable[16] ;
	unsigned int uint_channel_adc_offset[16] ;

// -----------



	unsigned int uint_channel_trigger_enable[16] ;
	unsigned int uint_channel_trigger_cfd_enable[16] ;
	unsigned int uint_channel_trigger_threshold[16] ;
	unsigned int uint_channel_trigger_peaking[16] ;
	unsigned int uint_channel_trigger_gap[16] ;
	unsigned int uint_channel_trigger_pulse_length[16] ;

	unsigned int uint_channel_he_trigger_threshold[16];
	unsigned int uint_channel_he_trigger_suppress;
	unsigned int uint_channel_he_trigger_generation_cfd_function_idx;

	unsigned int uint_sum_trigger_0_3;
	unsigned int uint_sum_trigger_4_7;
	unsigned int uint_sum_trigger_8_11;
	unsigned int uint_sum_trigger_12_15;

	unsigned int uint_pileup_window_length;
	unsigned int uint_re_pileup_window_length;

	unsigned int uint_vme_trigger_idx;
	unsigned int uint_vme_he_trigger_idx;
// -----------

	unsigned int uint_energy_peaking_value;
	unsigned int uint_energy_gap_value;
	unsigned int uint_energy_decay_tau_table;
	unsigned int uint_energy_decay_tau_factor;
	unsigned int uint_energy_average_factor;
	unsigned int uint_energy_pickup_idx;

	unsigned int uint_gate1_start_index;
	unsigned int uint_gate1_length;
	unsigned int uint_gate2_start_index;
	unsigned int uint_gate2_length;
	unsigned int uint_gate3_start_index;
	unsigned int uint_gate3_length;
	unsigned int uint_gate4_start_index;
	unsigned int uint_gate4_length;
	unsigned int uint_gate5_start_index;
	unsigned int uint_gate5_length;
	unsigned int uint_gate6_start_index;
	unsigned int uint_gate6_length;
	unsigned int uint_gate7_start_index;
	unsigned int uint_gate7_length;
	unsigned int uint_gate8_start_index;
	unsigned int uint_gate8_length;
	// -----------
	unsigned int uint_internal_sample_clock_idx;
	unsigned int uint_internal_sample_clock_fp_en;
	unsigned int uint_fp_clock_idx;
	unsigned int uint_sample_clock_idx;
	unsigned int uint_multiplier_idx;
	unsigned int uint_CoincidenceLookupMode_idx;
	// -----------

// -----------
	unsigned int uint_lemo_out_CO_select ;
	unsigned int uint_lemo_out_TO_select ;
	unsigned int uint_lemo_out_UO_select ;
// -----------






public:
	sis3316_get_configuration_parameters(void);

	int read_parameter_file(char *path);
	int write_parameter_file(char *path);

//	~sis3316_get_configuration_parameters(void);
};

#endif /* GET_CONFIGURATION_PARAMETERS_H_ */
