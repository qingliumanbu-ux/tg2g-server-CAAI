/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 副产品物料代码查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai10_inq_m)
int f_caai10_inq_m(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  sqlstr_1("");
	CString begin_time("");
	CString end_time("");
	CString mat_code("");
	CString sub_backlog_code("");

	int i =0;
	int do_type = 0 ;


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		do_type  = bcls_rec->Tables[0].Rows[0]["do_type"].ToDecimal().ToInt16();

		if(do_type == 2)
		{
			sub_backlog_code  = bcls_rec->Tables[0].Rows[0]["sub_backlog_code"].ToString();		
		}
		if(do_type == 1)//副产品中间品
		{
			sqlstr = "SELECT MAT_CODE,MAT_NAME,MAT_UNIT FROM TCAAC11"
				" WHERE VALID_FLAG = '1'"
				;

			sqlstr_1 = " select FIELD_NAME from tcaac14"
				       " where CODE = @sub_backlog_code"
					   ;
			cmd_inq1.SetCommandText(sqlstr_1);
			cmd_inq1.Parameters.Set("sub_backlog_code","PRO");
			cmd_inq1.ExecuteReader();		
			if(cmd_inq1.Read())
			{
				sqlstr = sqlstr + " AND " +cmd_inq1.GetString(1) + "= '1'" ;
			}
			cmd_inq1.Close();
			sqlstr = sqlstr + " order BY  mat_code";
				
		}
		else if(do_type == 2)//辅助材料
		{
			sqlstr = "SELECT MAT_CODE,MAT_NAME,MAT_UNIT as UNIT FROM TCAAC11"
				" WHERE VALID_FLAG = '1'"
				;
			sqlstr_1 = " select FIELD_NAME from tcaac14"
				       " where CODE = @sub_backlog_code"
					   ;
			cmd_inq1.SetCommandText(sqlstr_1);
			cmd_inq1.Parameters.Set("sub_backlog_code",sub_backlog_code);
			cmd_inq1.ExecuteReader();		
			if(cmd_inq1.Read())
			{
				sqlstr = sqlstr + " AND " +cmd_inq1.GetString(1) + "= '1'" ;
			}
			cmd_inq1.Close();
			sqlstr = sqlstr + " order BY  mat_code" ;
		}
		else if(do_type == 4)//产品代码
		{
			sqlstr = "SELECT MAT_CODE,MAT_NAME,MAT_UNIT FROM TCAAC11"
				" WHERE PROD_FLAG = '1'"
				" and VALID_FLAG = '1'"
				" order BY  mat_code"
				;	
		}
		else //所有生效的
		{
			sqlstr = "SELECT MAT_CODE,MAT_NAME,MAT_UNIT FROM TCAAC11"
				" WHERE VALID_FLAG = '1'"
				" order BY  mat_code"
				;
		}
		cmd_inq.SetCommandText(sqlstr);
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

