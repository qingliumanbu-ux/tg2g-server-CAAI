/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		ZHOULI
Version:    1.0
Date:		2013-2-9
Description:新增查询工序钢种规格下的单耗科目
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac05_1_inq)
//-EP_SYSTEM_HEAD_END

int f_caac05_1_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int i = 0 ;
	int fetchRowCount	= 0;
	CDecimal version_no = 1 ;

	CModel tcaac05("TCAAC05");

	
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		tcaac05["DEPT_CODE"] = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString().Trim();
		tcaac05["SUB_BACKLOG_CODE"] = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"			].ToString().Trim();	
		tcaac05["SG_SIGN"]          = bcls_rec->Tables[0].Rows[0]["SG_SIGN"					].ToString().Trim();
		tcaac05["MAT_THICK"]        = bcls_rec->Tables[0].Rows[0]["MAT_THICK"					].ToDecimal();
		tcaac05["MAT_WIDTH"]        = bcls_rec->Tables[0].Rows[0]["MAT_WIDTH"					].ToDecimal();
		tcaac05["MAT_LEN"]          = bcls_rec->Tables[0].Rows[0]["MAT_LEN"					].ToDecimal();

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"MAT_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"MAT_NAME");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SUB_BACKLOG_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"DEPT_CODE");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"SG_SIGN");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"MAT_THICK");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"MAT_WIDTH");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"MAT_LEN");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"VERSION_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"UNIT");


		//取最高版本号
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
				sqlstr = "	SELECT	NVL(max(VERSION_NO),0)+1"
					     " FROM	TCAAC05"
					     " WHERE SUB_BACKLOG_CODE = @sub_backlog_code"
						 " AND SG_SIGN            = @sg_sign"
						 " AND DEPT_CODE            = @dept_code"
						 " AND MAT_THICK          = @mat_thick"
						 " AND MAT_WIDTH          = @mat_width"
						 " AND MAT_LEN            = @mat_len"
					     ;
			break;
		} 	
			
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaac05["SUB_BACKLOG_CODE"] 	);      
		cmd_inq.Parameters.Set("dept_code", tcaac05["DEPT_CODE"]);
		cmd_inq.Parameters.Set("sg_sign"         , tcaac05["SG_SIGN"] 	);   
		cmd_inq.Parameters.Set("mat_thick"       , tcaac05["MAT_THICK"] 	);   
		cmd_inq.Parameters.Set("mat_width"       , tcaac05["MAT_WIDTH"] 	);   
		cmd_inq.Parameters.Set("mat_len"         , tcaac05["MAT_LEN"] 	);
		cmd_inq.ExecuteReader();
		if(cmd_inq.Read())
		{
			version_no = cmd_inq.GetDecimal(1);
		}
        cmd_inq.Close();
		
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
				sqlstr = "	SELECT	TCAAC03.MAT_CODE,TCAAC03.MAT_NAME,@dept_code,@sub_backlog_code,@sg_sign,@mat_thick,@mat_width,@mat_len,@version_no,tcaac11.mat_UNIT"
					     " FROM	TCAAC03,TCAAC11"
					     " WHERE SUB_BACKLOG_CODE = @sub_backlog_code"
						 " AND TCAAC11.MAT_CODE = TCAAC03.MAT_CODE"
					     ;
			break;
		} 	
			
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaac05["SUB_BACKLOG_CODE"] 	);      
		cmd_inq.Parameters.Set("sg_sign"         , tcaac05["SG_SIGN"] 	);   
		cmd_inq.Parameters.Set("dept_code", tcaac05["DEPT_CODE"]);
		cmd_inq.Parameters.Set("mat_thick"       , tcaac05["MAT_THICK"] 	);   
		cmd_inq.Parameters.Set("mat_width"       , tcaac05["MAT_WIDTH"] 	);   
		cmd_inq.Parameters.Set("mat_len"         , tcaac05["MAT_LEN"] 	);   
		cmd_inq.Parameters.Set("version_no"      , version_no 	);   
		cmd_inq.ExecuteReader();
		while(cmd_inq.Read())
		{
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["MAT_CODE"]         = cmd_inq.GetString(1);
			bcls_ret->Tables[0].Rows[i]["MAT_NAME"]         = cmd_inq.GetString(2);
			bcls_ret->Tables[0].Rows[i]["SUB_BACKLOG_CODE"] = cmd_inq.GetString(4);
			bcls_ret->Tables[0].Rows[i]["DEPT_CODE"] = cmd_inq.GetString(3);
			bcls_ret->Tables[0].Rows[i]["SG_SIGN"]          = cmd_inq.GetString(5);
			bcls_ret->Tables[0].Rows[i]["MAT_THICK"]        = cmd_inq.GetDecimal(6);
			bcls_ret->Tables[0].Rows[i]["MAT_WIDTH"]        = cmd_inq.GetDecimal(7);
			bcls_ret->Tables[0].Rows[i]["MAT_LEN"]          = cmd_inq.GetDecimal(8);
			bcls_ret->Tables[0].Rows[i]["VERSION_NO"]       = cmd_inq.GetDecimal(9);
			bcls_ret->Tables[0].Rows[i]["UNIT"]             = cmd_inq.GetString(10);
			i++;
		}          
        cmd_inq.Close();
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
