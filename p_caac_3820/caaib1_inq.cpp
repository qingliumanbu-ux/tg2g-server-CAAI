/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 备用表使用查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaib1_inq)
int f_caaib1_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  begin_time("");
	CString  end_time("");
	int i =0;
	CModel tcaaib1("TCAAIB1");

	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;

	try
	{
		try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 1000;
		}

		tcaaib1.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		if (bcls_rec->Tables[0].Columns.Contains("BEGIN_TIME"))
		{
			begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		}
		if (bcls_rec->Tables[0].Columns.Contains("END_TIME"))
		{
			end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();
		}

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "SELECT * FROM TCAAIB1"
				" where project_id =@project_id"
				;
			if (tcaaib1["BACK_C1"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C1 = @back_c1";
			if (tcaaib1["BACK_C2"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C2 = @back_c2";
			if (tcaaib1["BACK_C3"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C3 = @back_c3";
			if (tcaaib1["BACK_C4"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C4 = @back_c4";
			if (tcaaib1["BACK_C5"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C5 = @back_c5";
			if (begin_time.Trim() != "")sqlstr = sqlstr + " and AFFIRM_TIME >= @begin_time";
			if (end_time.Trim() != "")sqlstr = sqlstr + " and AFFIRM_TIME <= @end_time";
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("project_id", tcaaib1["PROJECT_ID"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c1", tcaaib1["BACK_C1"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c2", tcaaib1["BACK_C2"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c3", tcaaib1["BACK_C3"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c4", tcaaib1["BACK_C4"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c5", tcaaib1["BACK_C5"].ToString().Trim());
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
		cmd_inq.Close();


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "SELECT count(1) FROM TCAAIB1"
				" where project_id =@project_id"
				;
			if (tcaaib1["BACK_C1"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C1 = @back_c1";
			if (tcaaib1["BACK_C2"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C2 = @back_c2";
			if (tcaaib1["BACK_C3"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C3 = @back_c3";
			if (tcaaib1["BACK_C4"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C4 = @back_c4";
			if (tcaaib1["BACK_C5"].ToString().Trim() != "")sqlstr = sqlstr + " and BACK_C5 = @back_c5";
			if (begin_time.Trim() != "")sqlstr = sqlstr + " and AFFIRM_TIME >= @begin_time";
			if (end_time.Trim() != "")sqlstr = sqlstr + " and AFFIRM_TIME <= @end_time";
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("project_id", tcaaib1["PROJECT_ID"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c1", tcaaib1["BACK_C1"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c2", tcaaib1["BACK_C2"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c3", tcaaib1["BACK_C3"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c4", tcaaib1["BACK_C4"].ToString().Trim());
		cmd_inq.Parameters.Set("back_c5", tcaaib1["BACK_C5"].ToString().Trim());
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		v_total_count = cmd_inq.ExecuteScalar().ToInt32();


		bcls_ret->Tables.Add("PageInfo");
		bcls_ret->Tables["PageInfo"].Columns.Add(DT_INT32, "TotalRecordCount");
		bcls_ret->Tables["PageInfo"].Rows.Add();
		bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = v_total_count;

		cmd_inq.Close();

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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

