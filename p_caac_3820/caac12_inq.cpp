/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:物料价格查询:根据输入的查询条件，查询显示物料价格信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac12_inq)
//-EP_SYSTEM_HEAD_END

int f_caac12_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);


	int doFlag			= 0;
	int fetchRowCount	= 0;
	CString  mat_type("");

	CModel tcaac12("TCAAC12");


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
			pageInfo.PageSize   = 10000;
			EDLog(1,1,"pageInfo error[%s]",(const char*)ex.GetMsg());
		}  

		tcaac12.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		mat_type = bcls_rec->Tables[0].Rows[0]["mat_type"].ToString();

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "	SELECT	*"
					" FROM	TCAAC12"
					 " WHERE	MAT_CODE LIKE  '%'||@tcaac12.MAT_CODE||'%'"
					 " AND		MAT_NAME LIKE  '%'||@tcaac12.MAT_NAME||'%'"
					// " AND VALID_FLAG = @tcaac12.VALID_FLAG"
					 ;

			if(tcaac12["PRICE_TERMS"].ToString().Trim()	!=	"")
			{
				sqlstr	+=	" AND		PRICE_TERMS	=		@tcaac12.PRICE_TERMS "; 
			}
			if (tcaac12["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr += " AND		DEPT_CODE	=		@tcaac12.DEPT_CODE ";
			}
			if (mat_type.Trim() != "")
			{
				sqlstr	+=	" AND		mat_code in (select mat_code from tcaac11 where mat_type =@mat_type) "; 
			}
			sqlstr	+=	" ORDER	BY VALID_FLAG desc , VERSION_NO DESC	";

			break;
		}        		

		cmd_inq.SetCommandText(sqlstr);	
		cmd_inq.Parameters.Set("tcaac12.MAT_CODE"	, tcaac12["MAT_CODE"].ToString().Trim()		);	
		cmd_inq.Parameters.Set("tcaac12.MAT_NAME"	, tcaac12["MAT_NAME"].ToString().Trim()		);	
		cmd_inq.Parameters.Set("tcaac12.VALID_FLAG"	, tcaac12["VALID_FLAG"]	);	
		cmd_inq.Parameters.Set("tcaac12.DEPT_CODE", tcaac12["DEPT_CODE"]);
		Log::Trace("", __FUNCTION__, "产线=[{0}]  ", tcaac12["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_type", mat_type);
		if(tcaac12["PRICE_TERMS"].ToString().Trim()	!=	"")
		{
			cmd_inq.Parameters.Set("tcaac12.PRICE_TERMS"	, tcaac12["PRICE_TERMS"]	);
		}

		//cmd_inq.ExecuteQuery(bcls_ret->Tables[0],pageInfo.RecordFrom,pageInfo.PageSize);  
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]); 
		cmd_inq.Close();

		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr1 = "	SELECT	COUNT(1)\
					  FROM	TCAAC12\
					  WHERE	MAT_CODE LIKE  '%'||@tcaac12.MAT_CODE||'%'\
					  AND		MAT_NAME LIKE  '%'||@tcaac12.MAT_NAME||'%'\
					  ";

			if(tcaac12["PRICE_TERMS"].ToString().Trim()	!=	"")
			{
				sqlstr	+=	" AND		PRICE_TERMS	=		@tcaac12.PRICE_TERMS "; 
			}
			if (tcaac12["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr += " AND		DEPT_CODE	=		@tcaac12.DEPT_CODE ";
			}
			if (mat_type.Trim() != "")
			{
				sqlstr += " AND		mat_code in (select mat_code from tcaac03 where BACK_CODE_2 =@mat_type) ";
			}

			break;
		}        	
		cmd_inq1.SetCommandText(sqlstr1);	
		cmd_inq1.Parameters.Set("tcaac12.MAT_CODE"	, tcaac12["MAT_CODE"].ToString().Trim()		);
		cmd_inq1.Parameters.Set("tcaac12.MAT_NAME", tcaac12["MAT_NAME"].ToString().Trim());
		cmd_inq.Parameters.Set("tcaac12.DEPT_CODE", tcaac12["DEPT_CODE"]);
		cmd_inq1.Parameters.Set("mat_type", mat_type);
		if (tcaac12["PRICE_TERMS"].ToString().Trim() != "")
		{
			cmd_inq1.Parameters.Set("tcaac12.PRICE_TERMS", tcaac12["PRICE_TERMS"]);
		}

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