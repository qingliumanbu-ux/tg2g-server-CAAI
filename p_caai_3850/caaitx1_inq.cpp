#include "stdafx.h"
/// <summary>
/// Description: 产量查询[tcaai03]
/// Copyright: Baosight Software LTD.co Copyright (c) 2010
/// Company: 上海宝信软件股份有限公司
/// Author: 
/// Version: 1.0
/// History:
/// 2016-03-30  新建
/// </summary> 

// Service 入口
BM2F_ENTERACE(caaitx1_inq)
int f_caaitx1_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	int doFlag = 0;

	//定义变量
	int affectRow = 0;
	CString c_col = "";
	CString sqlstr = "";
	CString stats_period = "";
	CString sub_backlog_code = "";

	CModel tcaai03("TCAAI03");
	CDbCommand cmd_inq(conn);

	try
	{	

		//获取前台传入参数
		tcaai03.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		Log::Trace("", "", "stats_period = [{0}],DEPT_CODE = [{1}]", tcaai03["STATS_PERIOD"].ToString(), tcaai03["DEPT_CODE"].ToString());

		//按日及班次查产量
		bcls_ret->Tables[0].set_TableName("早班");
		sqlstr = " select stats_period as X1,sum(wt) Y1 from tcaai03 "
			" where 1=1 "
			;
		if (tcaai03["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and sub_backlog_code=@sub_backlog_code ";
		}			
			sqlstr = sqlstr + " and dept_code= @dept_code"
				" and prod_shift_no  = '1'"
				" and stats_period >= substr(@stats_period,1,6)||'01'"
				" and stats_period <= @stats_period"
			" group by stats_period "
			" order by stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai03["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai03["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("stats_period", tcaai03["STATS_PERIOD"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		bcls_ret->Tables.Add();
		bcls_ret->Tables[1].Clear();
		bcls_ret->Tables[1].set_TableName("晚班");
		sqlstr = " select stats_period as X1,sum(wt) Y1 from tcaai03 "
			" where 1=1 "
			;
		if (tcaai03["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and sub_backlog_code=@sub_backlog_code ";
		}
		sqlstr = sqlstr + " and dept_code= @dept_code"
			" and prod_shift_no  = '2'"
			" and stats_period >= substr(@stats_period,1,6)||'01'"
			" and stats_period <= @stats_period"
			" group by stats_period "
			" order by stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai03["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai03["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("stats_period", tcaai03["STATS_PERIOD"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_inq.Close();


		bcls_ret->Tables.Add();
		bcls_ret->Tables[2].Clear();
		bcls_ret->Tables[2].set_TableName("全天");
		sqlstr = " select stats_period as X1,sum(wt) Y1 from tcaai03 "
			" where 1=1 "
			;
		if (tcaai03["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			sqlstr = sqlstr + " and sub_backlog_code=@sub_backlog_code ";
		}
		sqlstr = sqlstr + " and dept_code= @dept_code"
			" and stats_period >= substr(@stats_period,1,6)||'01'"
			" and stats_period <= @stats_period"
			" group by stats_period "
			" order by stats_period "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai03["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai03["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("stats_period", tcaai03["STATS_PERIOD"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
		cmd_inq.Close();		

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = ex.GetMsg() + "\r\n" + sqlstr;
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;

}