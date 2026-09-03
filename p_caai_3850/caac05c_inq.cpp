/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-04-09
Description:bom 信息查询
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caac05c_inq)
//-EP_SYSTEM_HEAD_END

int f_caac05c_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CModel tcaac05c("TCAAC05C");

	CString  sqlstr("");
	CString  flag("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;
		try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch (CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 10000;
		}
		EDLog(1, 1, "pageInfo.RecordFrom[%d]pageInfo.PageSize[%d]", pageInfo.RecordFrom, pageInfo.PageSize);


		tcaac05c.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString();
		Log::Trace("", __FUNCTION__, "flag=[{0}]  ", flag);

		if (flag == "1")
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 通用
				sqlstr = "SELECT DISTINCT MAT_CODE AS PRODUCT_CODE FROM TCAAC05B WHERE 1=1 "
					" AND MAT_CODE NOT IN (SELECT PRODUCT_CODE FROM TCAAC05C) "
					;
				break;
			}

			Log::Trace("", __FUNCTION__, "sql=[{0}]  ", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
		}
		else
		{
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 通用
				sqlstr = " SELECT  * FROM TCAAC05C "
					" WHERE 1=1 ";
				if (tcaac05c["PRODUCT_CODE"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE=@product_code ";
				}
				if (tcaac05c["MAT_CODE"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND MAT_CODE=@mat_code ";
				}
				if (tcaac05c["PRODUCT_CODE_CNAME"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE_CNAME LIKE '%'||@product_code_cname||'%' ";
				}
				if (tcaac05c["SG_SIGN"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND SG_SIGN LIKE @sg_sign||'%' ";
				}
				if (tcaac05c["SG_STD"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND SG_STD LIKE @sg_std||'%' ";
				}
				sqlstr = sqlstr + " ORDER BY REC_CREATE_TIME DESC";

				break;
			}
			Log::Trace("", __FUNCTION__, "sql=[{0}]  ", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("product_code", tcaac05c["PRODUCT_CODE"].ToString());
			cmd_inq.Parameters.Set("mat_code", tcaac05c["MAT_CODE"].ToString());
			cmd_inq.Parameters.Set("product_code_cname", tcaac05c["PRODUCT_CODE_CNAME"].ToString());
			cmd_inq.Parameters.Set("sg_sign", tcaac05c["SG_SIGN"].ToString());
			cmd_inq.Parameters.Set("sg_std", tcaac05c["SG_STD"].ToString());
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0], pageInfo.RecordFrom, pageInfo.PageSize);
			cmd_inq.Close();

			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 通用
				sqlstr = "SELECT count(1) FROM TCAAC05C"
					" WHERE  1=1 "
					;
				if (tcaac05c["PRODUCT_CODE"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE=@product_code ";
				}
				if (tcaac05c["MAT_CODE"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND MAT_CODE=@mat_code ";
				}
				if (tcaac05c["PRODUCT_CODE_CNAME"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND PRODUCT_CODE_CNAME LIKE '%'||@product_code_cname||'%' ";
				}
				if (tcaac05c["SG_SIGN"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND SG_SIGN LIKE @sg_sign||'%' ";
				}
				if (tcaac05c["SG_STD"].ToString().Trim() != "")
				{
					sqlstr = sqlstr + " AND SG_STD LIKE @sg_std||'%' ";
				}
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("product_code", tcaac05c["PRODUCT_CODE"].ToString());
			cmd_inq.Parameters.Set("mat_code", tcaac05c["MAT_CODE"].ToString());
			cmd_inq.Parameters.Set("product_code_cname", tcaac05c["PRODUCT_CODE_CNAME"].ToString());
			cmd_inq.Parameters.Set("sg_sign", tcaac05c["SG_SIGN"].ToString());
			cmd_inq.Parameters.Set("sg_std", tcaac05c["SG_STD"].ToString());

			bcls_ret->Tables.Add("PageInfo");
			bcls_ret->Tables["PageInfo"].Columns.Add(DT_INT32, "TotalRecordCount");
			bcls_ret->Tables["PageInfo"].Rows.Add();
			bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = cmd_inq.ExecuteScalar().ToInt32();

			cmd_inq.Close();
		}

		


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}