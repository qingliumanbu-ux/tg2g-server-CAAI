/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:工序单耗信息查询:根据输入的查询条件，查询显示工序单耗信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac05_inq)
//-EP_SYSTEM_HEAD_END

int f_caac05_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CString		date_start				=	" ";
	CString		date_end				=	" ";
	CString v_dept_code("");

	CModel tcaac05("TCAAC05");

	
	CString  sqlstr("");
	CString  sqlstr1("");
	
	
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		try{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables[1].Rows[0]);
		}
		catch(CException& ex)
		{//2个变量信息是由前台的分页控件信息传入的，获取失败时,人工赋值一下。 
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize   = 5000;
			EDLog(1,1,"pageInfo error[%s]",(const char*)ex.GetMsg());
		}          

		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	获得输入参数
		////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
		tcaac05["SUB_BACKLOG_CODE"]	= bcls_rec->Tables[0].Rows[0]["sub_backlog_code"	].ToString().Trim();		
		tcaac05["MAT_CODE"]			= bcls_rec->Tables[0].Rows[0]["mat_code"			].ToString().Trim();	
		tcaac05["SG_SIGN"]				= bcls_rec->Tables[0].Rows[0]["sg_sign"				].ToString().Trim();	
		tcaac05["MAT_NAME"]			= bcls_rec->Tables[0].Rows[0]["mat_name"			].ToString().Trim();		
		tcaac05["VERSION_NO"]			= bcls_rec->Tables[0].Rows[0]["version_no"			].ToDecimal();	
		v_dept_code = bcls_rec->Tables[0].Rows[0]["dept_code"].ToString();
  		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
				sqlstr = "	SELECT	* "
					" FROM	TCAAC05 "
					" WHERE	SG_SIGN	 like '%'||@tcaac05.SG_SIGN||'%' "
					" AND	 MAT_CODE like '%'||@tcaac05.MAT_CODE||'%' "
					" AND	 mat_name like '%'||@tcaac05.MAT_NAME||'%' "				
					
					;
			if( tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() != "") sqlstr += " AND SUB_BACKLOG_CODE = @tcaac05.SUB_BACKLOG_CODE "; 
			if (tcaac05["VERSION_NO"].ToDecimal() != 0) sqlstr += " AND VERSION_NO = @tcaac05.VERSION_NO ";
			if (tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() == "" && v_dept_code.Trim() != "")sqlstr = sqlstr + " AND SUB_BACKLOG_CODE in (select sub_backlog_code from tcaac14 where CODE_LINE =@v_dept_code)";
			if (v_dept_code.Trim() != "") sqlstr += " AND DEPT_CODE = @v_dept_code ";
			break;
		} 		
		 	
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		if(tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() != "") cmd_inq.Parameters.Set("tcaac05.SUB_BACKLOG_CODE", tcaac05["SUB_BACKLOG_CODE"] );
		if( tcaac05["VERSION_NO"].ToDecimal() != 0) cmd_inq.Parameters.Set("tcaac05.VERSION_NO", tcaac05["VERSION_NO"] );
		
		cmd_inq.Parameters.Set("tcaac05.MAT_CODE"		, tcaac05["MAT_CODE"] 			);	
		cmd_inq.Parameters.Set("tcaac05.SG_SIGN"	, tcaac05["SG_SIGN"]		);
		cmd_inq.Parameters.Set("tcaac05.MAT_NAME"	, tcaac05["MAT_NAME"]	 	);
		cmd_inq.Parameters.Set("v_dept_code", v_dept_code);
            			
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);
            
        cmd_inq.Close();

		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
				sqlstr1 = "	SELECT	COUNT(1) "
					" FROM	TCAAC05 "
					" WHERE	SG_SIGN	 like '%'||@tcaac05.SG_SIGN||'%' "
					" AND	 MAT_CODE like '%'||@tcaac05.MAT_CODE||'%' "
					" AND	 MAT_NAME like '%'||@tcaac05.MAT_NAME||'%' "				
					
					;
				if (tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() != "") sqlstr1 += " AND SUB_BACKLOG_CODE = @tcaac05.SUB_BACKLOG_CODE ";
				if (tcaac05["VERSION_NO"].ToDecimal() != 0) sqlstr1 += " AND VERSION_NO = @tcaac05.VERSION_NO ";
				if (tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() == "" && v_dept_code.Trim() != "")sqlstr1 = sqlstr1 + " AND SUB_BACKLOG_CODE in (select sub_backlog_code from tcaac14 where CODE_LINE =@v_dept_code)";
				if (tcaac05["DEPT_CODE"].ToString().Trim() != "") sqlstr += " AND DEPT_CODE = @v_dept_code ";
			break;
		}            	
		cmd_inq1.SetCommandText(sqlstr1);
		
		if(tcaac05["SUB_BACKLOG_CODE"].ToString().Trim() != "") cmd_inq1.Parameters.Set("tcaac05.SUB_BACKLOG_CODE", tcaac05["SUB_BACKLOG_CODE"] );
		
		cmd_inq1.Parameters.Set("tcaac05.MAT_CODE"		, tcaac05["MAT_CODE"] 			);	
		cmd_inq1.Parameters.Set("tcaac05.SG_SIGN"	, tcaac05["SG_SIGN"]		);
		cmd_inq1.Parameters.Set("tcaac05.MAT_NAME"	, tcaac05["MAT_NAME"]	 	);
		cmd_inq.Parameters.Set("tcaac05.VERSION_NO", tcaac05["VERSION_NO"]);
		cmd_inq1.Parameters.Set("v_dept_code", v_dept_code);
		cmd_inq1.Close();		
    
		//返回的记录数。
		fetchRowCount = cmd_inq1.ExecuteScalar().ToInt32();


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