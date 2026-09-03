/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2012-10-15
Description: 执行核算
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
/*<remark>=========================================================

===========================================================</remark>*/ 
BM2_FUNCTION_EXPORT
int f_caai_xnmes(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_check1(CString stats_period, CString dept_code, CString begin_time, CString end_time, CDbConnection * conn);
//int f_caai_check2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caai_gb(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_pcost(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_ft(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_sj(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_cost(CString stats_period, CString dept_code, CString price_terms, CDbConnection * conn);
int f_caai_rate(CString stats_period, CString sub_backlog_code, CString divvy_type, CDbConnection * conn);
int f_caai_ny(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_start(CString dept_code, CString stats_period, CString begin_time, CString end_time, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	//程序用变量
	CString sqlstr("");
	int doFlag = 0;
	int i;
	int fetchRowCount = 0;
	int flag = 0;


	/* 实体类定义 */
	//CTCAAC06 tcaac06(conn);

	CDbCommand	cmd_inq(conn);
	CDbCommand	cmd_inq_sub(conn);

	
	try
	{
		/*获得传入参数*/

		//判断是否有核算期间，如果没有则加成本中心
		sqlstr = " select count(1)"
			" from tcaac06"
			" where stats_period = @stats_period"
			" and dept_code = @dept_code"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		if (cmd_inq.ExecuteScalar().ToInt32() == 0)
		{
			sqlstr = " insert into tcaac06(stats_period,S_DATETIME,E_DATETIME,VALID_FLAG,DEPT_CODE)"
				" values(@stats_period,@stats_period||'000000',@stats_period||'235959','1',@dept_code)"
				;
			cmd_inq_sub.SetCommandText(sqlstr);
			cmd_inq_sub.Parameters.Set("stats_period", stats_period);
			cmd_inq_sub.Parameters.Set("dept_code", dept_code);
			cmd_inq_sub.ExecuteNonQuery();
			cmd_inq_sub.Close();
		}

		//先收集
		doFlag = f_caai_xnmes(stats_period, dept_code,conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		doFlag = f_caai_check1(stats_period, dept_code, begin_time, end_time, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		doFlag = f_caai_gb(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		doFlag = f_caai_sj(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}		

		doFlag = f_caai_ft(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		doFlag = f_caai_ny(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		doFlag = f_caai_cost(stats_period, dept_code, "GJ", conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
		doFlag = f_caai_pcost(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}		
		
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);

		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
