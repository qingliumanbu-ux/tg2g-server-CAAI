/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:计划产量查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaijh_inq)
//-EP_SYSTEM_HEAD_END

int f_caaijh_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;


	CString		account_period = " ";
	CString		mat_code = " ";
	CString		shift_group = " ";
	CString		dept_code = " ";


	CModel tcaaia11("TCAAIA11");


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);

	try
	{
		tcaaia11.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		sqlstr = " select * from tcaaia11  "
			" where 1=1 ";
		if (tcaaia11["MAT_CODE"].ToString().Trim()!="")
			sqlstr = sqlstr + " and mat_code like '%" + tcaaia11["MAT_CODE"].ToString() + "%' ";
		if (tcaaia11["MAT_NAME"].ToString().Trim() != "")
			sqlstr = sqlstr + " and mat_name like '%" + tcaaia11["MAT_NAME"].ToString() + "%' ";
		if (tcaaia11["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			sqlstr = sqlstr + " and sub_backlog_code='" + tcaaia11["SUB_BACKLOG_CODE"].ToString() + "' ";
		if (tcaaia11["DEPT_CODE"].ToString().Trim() != "")
			sqlstr = sqlstr + " and dept_code='" + tcaaia11["DEPT_CODE"].ToString() + "' ";
		if (tcaaia11["PROD_DATE"].ToString().Trim() != "")
			sqlstr = sqlstr + " and prod_date='" + tcaaia11["PROD_DATE"].ToString() + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
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

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}