/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-11-20
Description: 物流回收信息查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "tcaaib1.h"
/******后台pc文件标准注释标记*****/

// service入口
BM2F_ENTERACE(caacjl_inq)

int f_caacjl_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	CDbCommand cmd(conn);
	CString  sqlstr("");

	int  v_total_count = 0;
	//系统的分页类信息。
	CPageInfo pageInfo;


	CDbCommand cmd_inq(conn);
	/* 实体类定义 */
	CModel tcaaib1("TCAAIB1");

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		tcaaib1.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		Log::Trace("", __FUNCTION__, "BACK_C1=[{0}]  ", tcaaib1["BACK_C1"].ToString());
		Log::Trace("", __FUNCTION__, "PROJECT_ID=[{0}]  ", tcaaib1["PROJECT_ID"].ToString());
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM TCAAIB1 WHERE 1=1  ";
			if (tcaaib1["BACK_C1"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND BACK_C1=@BACK_C1 ";
			}
			if (tcaaib1["PROJECT_ID"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PROJECT_ID=@PROJECT_ID ";
			}
			/*if (tcaacjl["THICK_MAX"].ToDecimal()>=0)
			{
			sqlstr = sqlstr + " AND THICK_MAX=@thick_max ";
			}
			if (tcaacjl["THICK_MIN"].ToDecimal()>=0)
			{
			sqlstr = sqlstr + " AND THICK_MIN=@thick_min ";
			}*/
			/*if (tcaacjl["OUT_UNIT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND OUT_UNIT_CODE=@out_unit_code ";
			}
			if (tcaacjl["RECV_DEPT"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND RECV_DEPT=@recv_dept ";
			}*/
			//sqlstr += "AND ARCHIVE_FLAG='0' ORDER BY BACK_C1,PROC_MODE,THICK_MIN  ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.Parameters.Set("BACK_C1", tcaaib1["BACK_C1"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("PROJECT_ID", tcaaib1["PROJECT_ID"].ToString().Trim());
		
		////cmd_inq.Parameters.Set("thick_max", tcaacjl["THICK_MAX"].ToDecimal());
		////cmd_inq.Parameters.Set("thick_min", tcaacjl["THICK_MIN"].ToDecimal());
		//cmd_inq.Parameters.Set("out_unit_code", tcaacjl["OUT_UNIT_CODE"].ToString().TrimOrBlank());
		//cmd_inq.Parameters.Set("recv_dept", tcaacjl["RECV_DEPT"].ToString().TrimOrBlank());
		//bcls_ret->Tables[0].set_TableName("Tcaacjl");
		//cmd_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
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
	return doFlag;
}

