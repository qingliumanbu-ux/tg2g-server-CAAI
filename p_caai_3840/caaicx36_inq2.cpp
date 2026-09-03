/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:查询机组明细信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx36_inq2)
//-EP_SYSTEM_HEAD_END

int f_caaicx36_inq2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;	
	int fetchRowCount	= 0;
	CModel tcaai04("TCAAI04");

	CString		begin_time = " ";
	CString		end_time = " ";


	CString  sqlstr("");
	
	
	CDbCommand cmd_inq(conn);
	
	try
	{
		begin_time = bcls_rec->Tables[1].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[1].Rows[0]["END_TIME"].ToString();
		tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);		
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句		
					
				sqlstr = " SELECT	mat_code,mat_name,sum(WT) WT,sum(COST_GJ) AMT,sum(COST_GJ) COST_GJ,sum(COST_YJ) COST_YJ ,sum(COST_CJ) COST_CJ"
					" ,decode(sum(WT),0,0,round(sum(COST_GJ)/sum(WT),6)) PRICE_UNIT_GJ"
					" ,decode(sum(WT),0,0,round(sum(COST_YJ)/sum(WT),6)) PRICE_UNIT_YJ"
					" ,decode(sum(WT),0,0,round(sum(COST_CJ)/sum(WT),6)) PRICE_UNIT_CJ"
					" FROM	tcaai04"
					" WHERE	1=1"
					" AND stats_period>=@begin_time"
					" and stats_period<=@end_time"
					" AND EQU_NO = @equ_no"
					" AND SUB_BACKLOG_CODE = @sub_backlog_code"
					" GROUP BY MAT_CODE,MAT_NAME"
					;
				
   			break;
		}  
		
		cmd_inq.SetCommandText(sqlstr);	
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));	
		cmd_inq.Parameters.Set("equ_no", tcaai04["EQU_NO"].ToString());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
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