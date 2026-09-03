/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-07-20
Description: 加工费信息查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
#include "tcaac16.h"
/******后台pc文件标准注释标记*****/

// service入口
BM2F_ENTERACE(caac16_inq)

int f_caac16_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	CDbCommand cmd(conn);
	CString  sqlstr("");

	int  v_total_count=0;
	//系统的分页类信息。
	CPageInfo pageInfo;


	CDbCommand cmd_inq(conn);
	/* 实体类定义 */
	CModel tcaac16("TCAAC16");

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		tcaac16.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		Log::Trace("", __FUNCTION__, "OUT_TASK_NO=[{0}]  ", tcaac16["OUT_TASK_NO"].ToString());
		/*try
		{
			pageInfo.MergeFrom(bcls_rec->Tables["PageInfo"].Rows[0]);
		}
		catch(CException& ce)
		{
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize   = 1000;
			EDLog(1,1,"pageInfo error[%s]",(const char*)ce.GetMsg());
		}
		EDLog(1,1,"pageInfo.RecordFrom[%d]pageInfo.PageSize[%d]",pageInfo.RecordFrom,pageInfo.PageSize);
*/
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tcaac16 WHERE 1=1  ";
			if (tcaac16["OUT_TASK_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND OUT_TASK_NO=@out_task_no ";
			}
			if (tcaac16["PROC_MODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PROC_MODE=@proc_mode ";
			}
			/*if (tcaac16["THICK_MAX"].ToDecimal()>=0)
			{
				sqlstr = sqlstr + " AND THICK_MAX=@thick_max ";
			}
			if (tcaac16["THICK_MIN"].ToDecimal()>=0)
			{
				sqlstr = sqlstr + " AND THICK_MIN=@thick_min ";
			}*/
			if (tcaac16["OUT_UNIT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND OUT_UNIT_CODE=@out_unit_code ";
			}
			if (tcaac16["RECV_DEPT"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND RECV_DEPT=@recv_dept ";
			}
			sqlstr += "AND ARCHIVE_FLAG='0' ORDER BY OUT_TASK_NO,PROC_MODE,THICK_MIN  ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.Parameters.Set("out_task_no",tcaac16["OUT_TASK_NO"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("proc_mode", tcaac16["PROC_MODE"].ToString().TrimOrBlank());
		//cmd_inq.Parameters.Set("thick_max", tcaac16["THICK_MAX"].ToDecimal());
		//cmd_inq.Parameters.Set("thick_min", tcaac16["THICK_MIN"].ToDecimal());
		cmd_inq.Parameters.Set("out_unit_code", tcaac16["OUT_UNIT_CODE"].ToString().TrimOrBlank());
		cmd_inq.Parameters.Set("recv_dept", tcaac16["RECV_DEPT"].ToString().TrimOrBlank());
		//bcls_ret->Tables[0].set_TableName("TCAAC16");
		//cmd_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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

