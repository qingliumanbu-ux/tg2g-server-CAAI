/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-04-09
Description: 会计期信息查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

/******后台pc文件标准注释标记*****/
/*<remark>=========================================================
/// <summary>
/// 岗位设定的信息查询
/// <para>
/// 1.根据输入参数查询相应信息。
///
/// <para>数据库表：tcaac15( 会计期信息表)         </para>
/// </summary>
/// <param name="ACCOUNT_PERIOD"> 会计期   </param>
/// <returns> 会计期信息</returns>
===========================================================</remark>*/
// service入口
BM2F_ENTERACE(caac15_inq)

int f_caac15_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString account_period("");

	int  v_total_count=0;
	//系统的分页类信息。
	CPageInfo pageInfo;


	CDbCommand cmd_inq(conn);
	CModel tcaac15("TCAAC15");

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		try
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


		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().Trim();
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tcaac15 WHERE 1=1  " ;

			if (account_period.Trim() != "")  sqlstr = sqlstr + " AND ACCOUNT_PERIOD=@account_period ";
			sqlstr = sqlstr +" ORDER BY ACCOUNT_PERIOD ASC ";

			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period.SubstringNE(0,6));
		Log::Trace("", "", "account_period.Substring(0,6)={0}", account_period.SubstringNE(0, 6));
		//定义返回的表名
		bcls_ret->Tables[0].set_TableName("TCAAC15");
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);
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

