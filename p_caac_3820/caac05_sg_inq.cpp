/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		ZHOULI
Version:    1.0
Date:		2013-2-9
Description:根据时间范围、工序取钢种
**************************************************/
//框架用头文件
#include "stdafx.h"

//业务用头文件


// service入口
BM2F_ENTERACE(caac05_sg_inq)
//-EP_SYSTEM_HEAD_END

int f_caac05_sg_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag			= 0;
	int i = 0 ;
	int fetchRowCount	= 0;

	CString begin_time = " ";
	CString end_time = " ";
	CString sub_backlog_code = " ";
	CString dept_code = " ";
	CString sg_sign = " ";
	CDecimal mat_thick = 0;
	CDecimal mat_width = 0;
	CDecimal mat_len = 0;
	CDecimal rowid = 1 ;

	CString v_table = "tcaai04";


	CDecimal version_no = 1 ;


	CModel tcaac05("TCAAC05");


	CString  sqlstr("");
	CString  sqlstr1("");

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq2(conn);

	try
	{
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString().Trim();
		sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"			].ToString().Trim();
		begin_time       = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"					].ToString().Trim();
		end_time         = bcls_rec->Tables[0].Rows[0]["END_TIME"					].ToString().Trim();

		Log::Trace("", __FUNCTION__, "sub_backlog_code = [{0}],begin_time = [{1}],end_time = [{2}],DEPT_CODE = [{3}]", (const char*)sub_backlog_code, (const char*)begin_time, (const char*)end_time,dept_code);

		////取工序所在的产线
		//sqlstr = " select CODE_LINE FROM TCAAC14"
		//	" WHERE SUB_BACKLOG_CODE = @sub_backlog_code"
		//	;
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
		//cmd_inq.Parameters.Set("begin_time", begin_time);
		//cmd_inq.Parameters.Set("end_time", end_time);
		//cmd_inq.ExecuteReader();
		//while (cmd_inq.Read())
		//{
		//	v_table = "tcaaia2" + cmd_inq.GetString(1);
		//}
		//cmd_inq.Close();

       bcls_ret->Tables[0].Columns.Add(DT_STRING,"SG_SIGN");
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "	SELECT distinct sg_sign"
				" FROM	" + v_table;
			sqlstr = sqlstr + " WHERE SUB_BACKLOG_CODE = @sub_backlog_code"
				" and dept_code=@dept_code "
				" AND PROD_DATE     >= @begin_time"
				" AND PROD_DATE     <= @end_time"
				;
			break;
		} 	

		cmd_inq2.SetCommandText(sqlstr);
		cmd_inq2.Parameters.Set("sub_backlog_code", sub_backlog_code 	); 
		cmd_inq2.Parameters.Set("dept_code", dept_code);
		cmd_inq2.Parameters.Set("begin_time"       , begin_time 	);   
		cmd_inq2.Parameters.Set("end_time"       , end_time 	);  
		cmd_inq2.ExecuteReader();
		while(cmd_inq2.Read())
		{
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["SG_SIGN"]         = cmd_inq2.GetString(1);
			i++;
		}
		cmd_inq2.Close();
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
