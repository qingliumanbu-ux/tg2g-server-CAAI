/*=========================================================================
//程序名称:     f_caai_check2
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：交易数据中消耗数据消耗按成本中心成本科目检测
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

BM2_FUNCTION_EXPORT
int f_caai_check2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;
	CString sqlstr = "";
	CString account_period = "";
	CString begin_time = "";
	CString end_time = "";
	CString dept_code = "";

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);

	try
	{
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"]; //获取成本核算会计期
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString(); //开始时间
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString(); //结束时间
		if (bcls_rec->Tables[0].Columns.Contains("DEPT_CODE"))
		{
			dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString(); //分厂
		}
		if (dept_code.Trim() == "")
		{
			dept_code = "%";
		}

		//更新成本中心成本科目
		sqlstr = "update tcaaia1"
			" set REC_REVISE_TIME = @datetime"
			" ,STATUS_FLAG='1'"
			" ,ERROR_INFO=' '"
			" ,COST_CENTER = (select COST_CENTER from tcaac08 where TCAAC08.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE)"
			" WHERE ACCOUNT_PERIOD = @account_period "
			" AND CHECK_FLAG='1'"
			" AND STATUS_FLAG in('0',' ') "
			" AND SYSTEM_ID like 'MM'||@dept_code"
			" AND (exists (SELECT 1 FROM TCAAC03 WHERE TCAAC03.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE and TCAAC03.MAT_CODE = tcaaia1.MAT_CODE ) OR  PRO_FLAG = 'P')"   //C 投入，P 产出，F 成品缴库，I 材料处置"
			" AND exists (select 1 from tcaac08 where TCAAC08.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE)"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		

		//2、判断是否工序对应成本中心 tcaac08
		sqlstr = "update tcaaia1"
			" set REC_REVISE_TIME = @datetime"
			" ,CHECK_FLAG='0'"
			" ,STATUS_FLAG='0'"
			" ,ERROR_INFO = '工序代码为【'||SUB_BACKLOG_CODE||'】，未维护成本中心，请至CAAC08画面维护!'"
			" WHERE ACCOUNT_PERIOD = @account_period "
			" AND CHECK_FLAG='1'"
			" AND STATUS_FLAG in('0',' ') "
			" AND SYSTEM_ID like 'MM'||@dept_code"
			" AND not exists (SELECT 1 FROM TCAAC08 WHERE TCAAC08.SUB_BACKLOG_CODE = tcaaia1.SUB_BACKLOG_CODE)"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datetime", CDateTime::Now().ToString("yyyyMMddHHmmss"));
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//3、成本中心与成本科目对应 tcaac04


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
