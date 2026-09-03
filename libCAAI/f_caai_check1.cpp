/*=========================================================================
//程序名称:     f_caai_check1
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：交易数据完整性检测：1、工序代码不能为空；2、物料代码必须有效；
// 3、判断是否有工序与物料代码是否生成收集规则 ；4、是否已经维护了固定价格
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

BM2_FUNCTION_EXPORT
int f_caai_check1(CString stats_period, CString dept_code, CString begin_time, CString end_time, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;
	CString sqlstr = "";
	CString heat_no = "";	

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_a1(conn);

	try
	{
		//原料、副产品、产成品的物料代码不能为空,工序代码不能为空
		sqlstr = "update tcaaia1 "
			" set STATS_PERIOD = @stats_period"
			" ,REC_REVISE_TIME = @datetime"
			" ,STATUS_FLAG = '0'"
			" ,CHECK_FLAG = '1'"
			" WHERE  1=1"			
			" AND (MAT_CODE IN (SELECT MAT_CODE FROM TCAAC11) or length(MAT_CODE)>19)"
			" AND SUB_BACKLOG_CODE !=' '"
			" AND CHECK_FLAG in ('0',' ')"
			" and dept_code = decode(trim(@dept_code),'',dept_code,@dept_code)"
			" AND PROD_TIME  between @begin_time AND @end_time"
			;
		cmd_inq_a1.SetCommandText(sqlstr);
		cmd_inq_a1.Parameters.Set("dept_code", dept_code);
		cmd_inq_a1.Parameters.Set("begin_time", begin_time);
		cmd_inq_a1.Parameters.Set("end_time", end_time);
		cmd_inq_a1.Parameters.Set("stats_period", stats_period);
		cmd_inq_a1.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq_a1.ExecuteNonQuery();
		cmd_inq_a1.Close();


		//工序代码为空的，提示
		sqlstr = "update tcaaia1 "
			" set STATS_PERIOD = @stats_period"
			" ,REC_REVISE_TIME = @datetime"
			" ,STATUS_FLAG = '0'"
			" ,CHECK_FLAG = '0'"
			" ,ERROR_INFO = '交易数据有误，工序代码不能为空！'"
			" WHERE 1=1"
			" and dept_code = decode(trim(@dept_code),'',dept_code,@dept_code)"
			" AND PROD_TIME  between @begin_time AND @end_time"
			" AND SUB_BACKLOG_CODE =' '"
			" AND CHECK_FLAG in('0', ' ')"
			;
		cmd_inq_a1.SetCommandText(sqlstr);
		cmd_inq_a1.Parameters.Set("dept_code", dept_code);
		cmd_inq_a1.Parameters.Set("begin_time", begin_time);
		cmd_inq_a1.Parameters.Set("end_time", end_time);
		cmd_inq_a1.Parameters.Set("stats_period", stats_period);
		cmd_inq_a1.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq_a1.ExecuteNonQuery();
		cmd_inq_a1.Close();

		//物料代码不存在，提示
		sqlstr = "update tcaaia1 "
			" set STATS_PERIOD = @stats_period"
			" ,REC_REVISE_TIME = @datetime"
			" ,STATUS_FLAG = '0'"
			" ,CHECK_FLAG = '0'"
			" ,ERROR_INFO = '交易数据有误，该物料代码在产副品代码表中不存在!'"
			" WHERE CHECK_FLAG in ('0',' ')"			
			" AND SUB_BACKLOG_CODE !=' '"
			" AND (MAT_CODE NOT IN (SELECT MAT_CODE FROM TCAAC11) or length(MAT_CODE)>19)"
			" and dept_code = decode(trim(@dept_code),'',dept_code,@dept_code)"
			" AND PROD_TIME  between @begin_time AND @end_time"
			;
		cmd_inq_a1.SetCommandText(sqlstr);
		cmd_inq_a1.Parameters.Set("dept_code", dept_code);
		cmd_inq_a1.Parameters.Set("begin_time", begin_time);
		cmd_inq_a1.Parameters.Set("end_time", end_time);
		cmd_inq_a1.Parameters.Set("stats_period", stats_period);
		cmd_inq_a1.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq_a1.ExecuteNonQuery();
		cmd_inq_a1.Close();

		//判断是否已经生成规则
		sqlstr = "update tcaaia1"
			" set REC_REVISE_TIME = @datetime"
			" ,STATUS_FLAG='1'"
			" ,ERROR_INFO=' '"
			" WHERE 1=1"
			" AND (exists (SELECT 1 FROM TCAAC03 WHERE TCAAC03.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE and TCAAC03.MAT_CODE = tcaaia1.MAT_CODE ) OR  PRO_FLAG = 'O' or length(mat_code)>19)"   //I 投入，P 产出，F 成品缴库，I 材料处置,产出坯料信息			
			" AND CHECK_FLAG='1'"
			" AND STATUS_FLAG in('0',' ') "
			" and dept_code = decode(trim(@dept_code),'',dept_code,@dept_code)"
			" AND STATS_PERIOD = @stats_period "
			
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//1、判断是否有工序与物料代码是否生成收集规则 tcaac03
		sqlstr = "update tcaaia1"
			" set REC_REVISE_TIME = @datetime"
			" ,CHECK_FLAG='0'"
			" ,STATUS_FLAG='0'"
			" ,ERROR_INFO = '工序代码为【'||SUB_BACKLOG_CODE||'】，物料代码为【'||MAT_CODE||'】未维护收集规则，请至CAAC03画面维护!'"
			" WHERE 1=1"			
			" AND CHECK_FLAG='1'"
			" AND STATUS_FLAG in('0',' ') "
			" and  length(MAT_CODE)<19"
			" AND PRO_FLAG = 'I'"   //I 投入，O 产出，F 成品缴库，I 材料处置
			" AND not exists (SELECT 1 FROM TCAAC03 WHERE TCAAC03.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE and TCAAC03.MAT_CODE = tcaaia1.MAT_CODE ) "
			" and dept_code = decode(trim(@dept_code),'',dept_code,@dept_code)"
			" AND STATS_PERIOD = @stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//1、把未生成收集规则的物料代码新增到 TCAAC03 
		sqlstr = "insert into tcaac03"
			"(BACK_CODE_1,SUB_BACKLOG_CODE,MAT_CODE,MAT_NAME)"
			" select distinct 'S',sub_backlog_code,mat_code,mat_name from  tcaaia1 "
			" WHERE 1=1 "
			" AND CHECK_FLAG = '0' "
			" AND STATUS_FLAG in('0', ' ') "
			" and  length(MAT_CODE)<19 "
			" AND PRO_FLAG = 'I' "
			" AND not exists(SELECT 1 FROM TCAAC03 WHERE TCAAC03.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE and TCAAC03.MAT_CODE = tcaaia1.MAT_CODE  and back_code_1 = 'S') "
			" and sub_backlog_code in('E', 'L', 'R', 'C') "
			" AND stats_period = @stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		//判断是否已经维护了物料价格
		sqlstr = "update tcaaia1"
			" set REC_REVISE_TIME = @datetime"
			" ,CHECK_FLAG='0'"
			" ,STATUS_FLAG='0'"
			" ,ERROR_INFO = '物料代码为【'||MAT_CODE||'】未维护固定价格，请至CAAC12画面维护!'"
			" WHERE 1=1"
			" AND CHECK_FLAG='1'"
			" AND STATUS_FLAG in('0',' ') "
			" AND PRO_FLAG = 'I'"   //I 投入，P 产出，F 成品缴库，I 材料处置
			" and length(MAT_CODE)<19"
			" AND not exists (SELECT 1 FROM TCAAC12 WHERE  TCAAC12.MAT_CODE = tcaaia1.MAT_CODE )"
			" AND MAT_CODE NOT LIKE '%'||'#'||'%'"
			" AND STATS_PERIOD = @stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//更新成本中心为工序
		sqlstr = "update tcaaia1"
			" set cost_center = sub_backlog_code"			
			" WHERE 1=1"
			" AND CHECK_FLAG='1'"
			" and dept_code = decode(trim(@dept_code), '', dept_code, @dept_code)"
			" and STATS_PERIOD = @stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//判断有投无产投产
		sqlstr = " update tcaaia1 t1"
			" set REC_REVISE_TIME = @datetime"
			" ,CHECK_FLAG='0'"
			" ,STATUS_FLAG='0'"
			" ,ERROR_INFO = '批次号为【'||RELATION_NO||'】，工序为【'||SUB_BACKLOG_CODE||'】有投无产!'"
			" WHERE 1=1"
			" AND CHECK_FLAG='1'"
			//" AND STATUS_FLAG in('0',' ') "
			" AND PRO_FLAG = 'I'"   
			" AND not exists (SELECT 1 FROM tcaaia1 t2 WHERE  t1.RELATION_NO = t2.RELATION_NO and t1.sub_Backlog_code = t2.sub_Backlog_code and t1.stats_period = t2.stats_period and  PRO_FLAG = 'O'  and dept_code = decode(trim(@dept_code), '', dept_code, @dept_code) and  t2.STATS_PERIOD = @stats_period )"
			" and dept_code = decode(trim(@dept_code), '', dept_code, @dept_code)"
			" and STATS_PERIOD = @stats_period "			
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//判断有产无投
		sqlstr = " update tcaaia1 t1"
			" set REC_REVISE_TIME = @datetime"
			" ,ERROR_INFO = '【警告】批次号为【'||RELATION_NO||'】，工序为【'||SUB_BACKLOG_CODE||'】有产无投!'"
			" WHERE 1=1"
			" AND CHECK_FLAG='1'"
			//" AND STATUS_FLAG in('0',' ') "
			" AND PRO_FLAG = 'O'"
			" AND not exists (SELECT 1 FROM tcaaia1 t2 WHERE  t1.RELATION_NO = t2.RELATION_NO and t1.sub_Backlog_code = t2.sub_Backlog_code and t1.stats_period = t2.stats_period and  PRO_FLAG = 'I'  and dept_code = decode(trim(@dept_code), '', dept_code, @dept_code) and  t2.STATS_PERIOD = @stats_period )"
			" and dept_code = decode(trim(@dept_code), '', dept_code, @dept_code)"
			" and STATS_PERIOD = @stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
