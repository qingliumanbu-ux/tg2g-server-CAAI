/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 产副品信息查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac04_inq)
//-EP_SYSTEM_HEAD_END

int f_caac04_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;	
	
	CModel tcaac11("TCAAC11");

	CString  sqlstr("");
	CDbCommand cmd_inq(conn);
	
	try
	{
		tcaac11.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句		
				  				
			sqlstr = "	SELECT	*"
					" FROM	TCAAC11 "
					" WHERE	MAT_CODE LIKE  '%'||@tcaac11.MAT_CODE||'%' "
					" AND MAT_NAME LIKE  '%'||@tcaac11.MAT_NAME||'%' "
					" AND VALID_FLAG LIKE  '%'||@tcaac11.VALID_FLAG "
					;
					if(tcaac11["MAT_TYPE"].ToString().Trim()!="")
						sqlstr = sqlstr + " AND MAT_TYPE = @tcaac11.MAT_TYPE " ;


					sqlstr = sqlstr + " ORDER	BY  MAT_TYPE,mat_code" ;
					
   			break;
		}  
		
		cmd_inq.SetCommandText(sqlstr);				
		cmd_inq.Parameters.Set("tcaac11.MAT_CODE"	, tcaac11["MAT_CODE"].ToString().Trim()	); 
		cmd_inq.Parameters.Set("tcaac11.MAT_NAME", tcaac11["MAT_NAME"].ToString().Trim());
		cmd_inq.Parameters.Set("tcaac11.VALID_FLAG", tcaac11["VALID_FLAG"].ToString().Trim());
		cmd_inq.Parameters.Set("tcaac11.MAT_TYPE", tcaac11["MAT_TYPE"].ToString().Trim());
            		
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