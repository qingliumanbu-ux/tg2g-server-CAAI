/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 权重系数查询
**************************************************/

#include "stdafx.h"

// Service 入口
BM2F_ENTERACE(caac09_inq)

int f_caac09_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//APP_BEGIN()
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义
	int doFlag = 0;

	CString sqlstr("");

	/* 实体类定义 */
	CModel tcaac09("TCAAC09");

	CDbCommand cmd_inq(conn);

	try
	{
		/* 获得输入参数 */
		tcaac09.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT * "
				" FROM tcaac09"
				" WHERE 1=1"
				;
			if (tcaac09["SUB_BACKLOG_CODE"].ToString().Trim() != "")sqlstr = sqlstr + " AND SUB_BACKLOG_CODE = @tcaac09.SUB_BACKLOG_CODE";
			if (tcaac09["ITEM_DIV"].ToString().Trim() != "") sqlstr = sqlstr + " AND ITEM_DIV = @tcaac09.ITEM_DIV";
			sqlstr = sqlstr + " ORDER BY MAT_THICK,MAT_WIDTH,MAT_LEN";
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tcaac09.SUB_BACKLOG_CODE", tcaac09["SUB_BACKLOG_CODE"]);
		cmd_inq.Parameters.Set("tcaac09.ITEM_DIV", tcaac09["ITEM_DIV"]);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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
