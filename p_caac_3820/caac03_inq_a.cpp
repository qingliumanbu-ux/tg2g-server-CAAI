/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:根据工序代码查询未设置分摊规则的物料代码
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac03_inq_a)
//-EP_SYSTEM_HEAD_END

int f_caac03_inq_a(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	
	CTracer log(__FUNCTION__);	
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CModel tcaac03("TCAAC03");

	
	CString  sqlstr("");
	CString v_code = "";
	

	CDbCommand cmd_inq(conn);
	
	try
	{
		
		
		tcaac03.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		////获取是哪个使用字段使用
		//sqlstr = "select CODE from tcaac14"
		//	" where sub_backlog_code = @tcaac03.SUB_BACKLOG_CODE"
		//	;
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("tcaac03.SUB_BACKLOG_CODE", tcaac03.SUB_BACKLOG_CODE);
		//cmd_inq.ExecuteReader();
		//if (cmd_inq.Read())
		//{
		//	v_code = cmd_inq.GetString(1);
		//}
		//cmd_inq.Close();

		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
				sqlstr = "	SELECT	MAT_CODE,MAT_NAME,'" + tcaac03["SUB_BACKLOG_CODE"].ToString() + "' as  SUB_BACKLOG_CODE,'" + tcaac03["BACK_CODE_1"].ToString() + "' as BACK_CODE_1"
					" FROM	TCAAC11 "
					" WHERE	  MAT_CODE NOT IN (SELECT MAT_CODE FROM TCAAC03 WHERE SUB_BACKLOG_CODE	= '" + tcaac03["SUB_BACKLOG_CODE"].ToString() + "' and back_code_1='" + tcaac03["BACK_CODE_1"].ToString() + "' )"
					"  and VALID_FLAG='1' "
					;
				//if (v_code.Trim() != "")
				//	sqlstr = sqlstr +" and " + v_code + "= '1'"; //类似于use_flag_1 = '1'
					; 
			
    		break;
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