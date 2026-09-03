/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 成本中心查询
**************************************************/


#include "stdafx.h"


// service入口
BM2F_ENTERACE(caac01_inq)
//-EP_SYSTEM_HEAD_END
int f_caac01_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn)
{

	/*定义函数名*/
	CTracer log(__FUNCTION__);


	/*程序用变量*/
	int doFlag = 0;
	int fetchRowCount;
	
	CString  sqlstr = "";

	//系统的分页类信息。
	CPageInfo pageInfo;

	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);	
	CModel tcaac01("TCAAC01");
	 
	try
	{	

		tcaac01.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:
			sqlstr = " SELECT * "
				" FROM TCAAC01 where 1=1"
				;
			if (tcaac01["COST_CENTER"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and COST_CENTER=@cost_center ";
			}
			if (tcaac01["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " and DEPT_CODE=@dept_code ";
			}
			sqlstr = sqlstr + " ORDER BY COST_CENTER";
				;			
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("cost_center", tcaac01["COST_CENTER"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaac01["DEPT_CODE"].ToString());
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
